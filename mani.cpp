// ===========================================================
// ⚡️ KRYSTALLENGINE COMPLETE SCENE
// Day/Night + Model + Camera + Trees + Birds + Rain/Snow
// ===========================================================

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cassert>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glut.h>
#include <ctime>
#include <cstdlib>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

// ===========================
// ⚡️ Structs
// ===========================
struct Model {
    GLuint vao, vbo, ebo;
    size_t indexCount;
    void draw() {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, 0);
    }
};
struct Shader {
    GLuint id;
};
struct Color {
    float r, g, b;
};
struct Vec3 {
    float x, y, z;
};
struct Camera {
    float yaw, pitch;
    Vec3 position;

    Camera() : yaw(-90.0f), pitch(0.0f), position({0.0f, 1.5f, 3.0f}) {}
};
struct Raindrop { float x, y, speed; };
struct Snowflake { float x, y, speed; };
struct Star { float x, y; };
struct Bird { float x, y; };
struct Tree {
    float x, y;

    void draw(float windOffset) {
        glColor3f(0.55f, 0.27f, 0.07f);
        glBegin(GL_QUADS);
        glVertex2f(x - 0.01f, y);
        glVertex2f(x + 0.01f, y);
        glVertex2f(x + 0.01f, y + 0.1f);
        glVertex2f(x - 0.01f, y + 0.1f);
        glEnd();

        drawCircle(x + windOffset, y + 0.1f, 0.03f, 0.0f, 0.8f, 0.0f);
        drawCircle(x + windOffset - 0.03f, y + 0.08f, 0.025f, 0.0f, 0.8f, 0.0f);
        drawCircle(x + windOffset + 0.03f, y + 0.08f, 0.025f, 0.0f, 0.8f, 0.0f);
    }

    void drawCircle(float cx, float cy, float r, float red, float green, float blue) {
        glColor3f(red, green, blue);
        glBegin(GL_TRIANGLE_FAN);
        for(int i = 0; i <= 360; ++i) {
            float degInRad = i * M_PI / 180.0f;
            glVertex2f(cx + cos(degInRad) * r, cy + sin(degInRad) * r);
        }
        glEnd();
    }
};

Camera camera;
float dayTime = 12.0f;

void updateTime(float deltaTime) {
    dayTime += deltaTime * 0.1f;
    if (dayTime >= 24.0f) dayTime = 0.0f;
}

Color getSkyColor() {
    if (dayTime >= 6.0f && dayTime < 18.0f) {
        return {0.53f, 0.8f, 1.0f};
    } else {
        return {0.0f, 0.0f, 0.1f};
    }
}

// ===========================
// ⚡️ Scene Objects
// ===========================
bool isRaining = true;
bool isSnowing = false;

std::vector<Raindrop> raindrops;
std::vector<Snowflake> snowflakes;
std::vector<Star> stars;
std::vector<Bird> birds;
std::vector<Tree> trees;

float windOffset = 0.0f;

void initScene() {
    srand(time(nullptr));

    trees = {
        {-0.5f, -0.7f},
        {-0.2f, -0.65f},
        {0.3f, -0.75f},
        {0.7f, -0.7f}
    };
    for(int i = 0; i < 100; ++i) {
        raindrops.push_back({(float)rand()/RAND_MAX * 2.0f - 1.0f,
                             (float)rand()/RAND_MAX,
                             0.005f});
    }
    for(int i = 0; i < 100; ++i) {
        snowflakes.push_back({(float)rand()/RAND_MAX * 2.0f - 1.0f,
                              (float)rand()/RAND_MAX,
                              0.002f});
    }
    for(int i = 0; i < 100; ++i) {
        stars.push_back({(float)rand()/RAND_MAX * 2.0f - 1.0f,
                         (float)rand()/RAND_MAX});
    }
    for(int i = 0; i < 5; ++i) {
        birds.push_back({-1.0f + (float)(i) * 0.3f,
                          0.5f + (float)(rand()%100)/300.0f});
    }
}

// ===========================
// ⚡️ Draw Objects
// ===========================
void drawRaindrops() {
    for (auto &rd : raindrops) {
        rd.y -= rd.speed;
        if (rd.y < -1.0f) rd.y = 1.0f;

        glColor3f(0.5f, 0.5f, 1.0f);
        glBegin(GL_LINES);
        glVertex2f(rd.x, rd.y);
        glVertex2f(rd.x, rd.y + 0.01f);
        glEnd();
    }
}

void drawSnowflakes() {
    for (auto &snow : snowflakes) {
        snow.y -= snow.speed;
        if (snow.y < -1.0f) snow.y = 1.0f;

        glPointSize(3.0f);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_POINTS);
        glVertex2f(snow.x, snow.y);
        glEnd();
    }
}

void drawStars() {
    for (auto &star : stars) {
        glPointSize(2.0f);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_POINTS);
        glVertex2f(star.x, star.y);
        glEnd();
    }
}

void drawBirds() {
    for (auto &bird : birds) {
        bird.x += 0.001f;
        if (bird.x > 1.0f) bird.x = -1.0f;

        glColor3f(0.1f, 0.1f, 0.1f);
        glBegin(GL_LINES);
        glVertex2f(bird.x - 0.01f, bird.y);
        glVertex2f(bird.x, bird.y + 0.005f);
        glVertex2f(bird.x, bird.y + 0.005f);
        glVertex2f(bird.x + 0.01f, bird.y);
        glEnd();
    }
}

// ===========================
// ⚡️ Main Loop
// ===========================
void display() {
    Color sky = getSkyColor();
    glClearColor(sky.r, sky.g, sky.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Kechasi yulduzlar
    if (dayTime >= 18.0f || dayTime < 6.0f) {
        drawStars();
    }

    // Daraxtlar
    for (auto &t : trees) {
        t.draw(windOffset);
    }

    // Qushlar
    drawBirds();

    // Ob-havo
    if (isRaining) drawRaindrops();
    if (isSnowing) drawSnowflakes();

    glutSwapBuffers();
}

void timer(int value) {
    windOffset = 0.01f * sin(value * M_PI / 180.0f);
    glutPostRedisplay();
    glutTimerFunc(16, timer, value + 1);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutCreateWindow("KrystallEngine Demo");
    glutDisplayFunc(display);
    glutTimerFunc(16, timer, 0);
    initScene();
    glutMainLoop();
    return 0;
}