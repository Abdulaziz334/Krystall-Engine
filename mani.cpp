#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include "stb_image.h"           // Rasmlarni yuklash
#include "obj_loader.h"           // .obj model yuklash (oddiy loader)

struct Vector3 {
    float x, y, z;
};
struct Bird {
    Vector3 pos;
    float speed;
};
struct Tree {
    Vector3 pos;
};
struct Raindrop {
    Vector3 pos;
    float speed;
};
struct Snowflake {
    Vector3 pos;
    float speed;
};
struct Star {
    Vector3 pos;
};
struct Camera {
    float x, y, z, yaw, pitch;
};

Camera camera = {0.0f, 2.0f, 5.0f, -90.0f, 0.0f};
float dayTime = 12.0f;

std::vector<Raindrop> raindrops;
std::vector<Snowflake> snowflakes;
std::vector<Star> stars;
std::vector<Bird> birds;
std::vector<Tree> trees;

bool isRaining = true;
bool isSnowing = false;

GLuint textureId;
OBJModel treeModel;

void loadTexture(const char* path) {
    int w, h, channels;
    unsigned char* data = stbi_load(path, &w, &h, &channels, 3);
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0,
                GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
}

void initScene() {
    srand(time(nullptr));
    loadTexture("resources/tree.png");        // Daraxt uchun
    treeModel = loadOBJ("resources/tree.obj"); // .obj modeli

    for(int i = 0; i < 100; ++i) {
        raindrops.push_back({{((float)rand()/RAND_MAX-0.5f) * 10.0f, 5.0f, ((float)rand()/RAND_MAX-0.5f) * 10.0f}, 0.1f});
        snowflakes.push_back({{((float)rand()/RAND_MAX-0.5f) * 10.0f, 5.0f, ((float)rand()/RAND_MAX-0.5f) * 10.0f}, 0.03f});
    }
    for(int i = 0; i < 100; ++i) {
        stars.push_back({{((float)rand()/RAND_MAX-0.5f) * 30.0f, ((float)rand()/RAND_MAX) * 15.0f + 5.0f, ((float)rand()/RAND_MAX-0.5f) * 30.0f}});
    }
    for(int i = 0; i < 5; ++i) {
        birds.push_back({{-5.0f + i * 2.0f, 3.0f, ((float)rand()/RAND_MAX-0.5f) * 5.0f, 0.01f}});
    }
    trees = {
        {{2.0f, 0.0f, -3.0f}},
        {{-2.5f, 0.0f, -4.0f}},
        {{1.0f, 0.0f, -6.0f}},
        {{-1.5f, 0.0f, -7.0f}}
    };
}

// Yorug‘lik sozlash
void setupLights() {
    GLfloat light_position[] = {1.0f, 5.0f, 1.0f, 1.0f};
    GLfloat ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void drawGround() {
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.3f, 0.7f, 0.3f);
    glBegin(GL_QUADS);
    glVertex3f(-100.0f, 0.0f, -100.0f);
    glVertex3f(-100.0f, 0.0f, 100.0f);
    glVertex3f(100.0f, 0.0f, 100.0f);
    glVertex3f(100.0f, 0.0f, -100.0f);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

void drawTrees() {
    glBindTexture(GL_TEXTURE_2D, textureId);
    for (auto &t : trees) {
        glPushMatrix();
        glTranslatef(t.pos.x, t.pos.y, t.pos.z);
        drawOBJ(treeModel);
        glPopMatrix();
    }
}

void drawRain() {
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.5f, 0.5f, 1.0f);
    glBegin(GL_LINES);
    for (auto &rd : raindrops) {
        glVertex3f(rd.pos.x, rd.pos.y, rd.pos.z);
        glVertex3f(rd.pos.x, rd.pos.y + 0.1f, rd.pos.z);
    }
    glEnd();
}

void drawSnow() {
    glPointSize(3.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POINTS);
    for (auto &snow : snowflakes) {
        glVertex3f(snow.pos.x, snow.pos.y, snow.pos.z);
    }
    glEnd();
}

void drawStars() {
    glPointSize(2.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POINTS);
    for (auto &star : stars) {
        glVertex3f(star.pos.x, star.pos.y, star.pos.z);
    }
    glEnd();
}

void drawBirds() {
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.1f, 0.1f, 0.1f);
    for (auto &bird : birds) {
        glPushMatrix();
        glTranslatef(bird.pos.x, bird.pos.y, bird.pos.z);
        glBegin(GL_TRIANGLES);
        glVertex3f(-0.1f, 0.0f, 0.0f);
        glVertex3f(0.1f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.1f, 0.0f);
        glEnd();
        glPopMatrix();
    }
}

void updateScene() {
    for (auto &rd : raindrops) {
        rd.pos.y -= rd.speed;
        if (rd.pos.y < 0.0f) rd.pos.y = 5.0f;
    }
    for (auto &snow : snowflakes) {
        snow.pos.y -= snow.speed;
        if (snow.pos.y < 0.0f) snow.pos.y = 5.0f;
    }
    for (auto &bird : birds) {
        bird.pos.x += bird.speed;
        if (bird.pos.x > 5.0f) bird.pos.x = -5.0f;
    }
}

void setSkyColor() {
    float t = dayTime;

    float r, g, b;

    if (t >= 6.0f && t < 18.0f) {
        r = 0.53f; g = 0.8f; b = 1.0f;
    } else {
        r = 0.0f; g = 0.0f; b = 0.1f;
    }
    glClearColor(r, g, b, 1.0f);
}

void display() {
    setSkyColor();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glRotatef(camera.pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(camera.yaw, 0.0f, 1.0f, 0.0f);
    glTranslatef(-camera.x, -camera.y, -camera.z);

    setupLights();
    drawGround();
    drawTrees();
    drawBirds();
    if (isRaining) drawRain();
    if (isSnowing) drawSnow();
    if (dayTime >= 18.0f || dayTime < 6.0f) drawStars();

    glutSwapBuffers();
}

void timer(int value) {
    dayTime += 0.01f;
    if (dayTime >= 24.0f) dayTime = 0.0f;

    updateScene();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void keyboard(unsigned char key, int, int) {
    float moveSpeed = 0.1f;

    switch (key) {
        case 'w': camera.z -= moveSpeed; break;
        case 's': camera.z += moveSpeed; break;
        case 'a': camera.x -= moveSpeed; break;
        case 'd': camera.x += moveSpeed; break;
        case 'q': camera.y += moveSpeed; break;
        case 'e': camera.y -= moveSpeed; break;
        case 27: exit(0);
    }
}

void specialKeys(int key, int, int) {
    float angleSpeed = 2.0f;

    switch (key) {
        case GLUT_KEY_LEFT: camera.yaw -= angleSpeed; break;
        case GLUT_KEY_RIGHT: camera.yaw += angleSpeed; break;
        case GLUT_KEY_UP: camera.pitch += angleSpeed; break;
        case GLUT_KEY_DOWN: camera.pitch -= angleSpeed; break;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(1024, 768);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("KrystallEngine 3D Demo - Final Version");

    glewInit();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    initScene();

    glutDisplayFunc(display);
    glutTimerFunc(16, timer, 0);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    glutMainLoop();
    return 0;
}