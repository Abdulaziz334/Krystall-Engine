// ===========================================================
// ⚡ KrystallEngine Demo (3D) - Asosiy Fayl
// Min: Model, Kamera, Yorug'lik, 3D Render
// ===========================================================

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cassert>
#include <map>
#include <memory>
#include <sstream>
#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct App {
    static float lastX, lastY;
    static bool firstMouse;
};
float App::lastX = 512, App::lastY = 384;
bool App::firstMouse = true;

struct Camera {
    glm::vec3 Position;
    float Yaw, Pitch, Speed;

    Camera(glm::vec3 pos) : Position(pos), Yaw(-90.0f), Pitch(0.0f), Speed(2.5f) {}

    glm::mat4 GetViewMatrix() {
        glm::vec3 direction;
        direction.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        direction.y = sin(glm::radians(Pitch));
        direction.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        return glm::lookAt(Position, Position + glm::normalize(direction), glm::vec3(0.0f,1.0f,0.0f));
    }
};
Camera camera(glm::vec3(0.0f, 1.5f, 3.0f));

// ===========================================================
// ⚡ UTILITY FUNKSIYALAR
// ===========================================================
static std::string loadFile(const std::string &path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Fayl o'qilmadi: " << path << '\n';
        return {};
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}
void error_callback(int error, const char* description) {
    std::cerr << "GLFW Xato: " << description << '\n';
}

// ===========================================================
// ⚡ SHADER
// ===========================================================
struct Shader {
    unsigned int id;

    Shader(const std::string &vertSrc, const std::string &fragSrc) {
        unsigned int vShader = glCreateShader(GL_VERTEX_SHADER);
        const char *vCode = vertSrc.c_str();
        glShaderSource(vShader, 1, &vCode, NULL);
        glCompileShader(vShader);
        int success;
        glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(vShader, 512, NULL, log);
            std::cerr << "Vertex Shader Xato:\n" << log << '\n';
        }

        unsigned int fShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char *fCode = fragSrc.c_str();
        glShaderSource(fShader, 1, &fCode, NULL);
        glCompileShader(fShader);
        glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(fShader, 512, NULL, log);
            std::cerr << "Fragment Shader Xato:\n" << log << '\n';
        }

        id = glCreateProgram();
        glAttachShader(id, vShader);
        glAttachShader(id, fShader);
        glLinkProgram(id);
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success) {
            char log[512];
            glGetProgramInfoLog(id, 512, NULL, log);
            std::cerr << "Shader Link Xato:\n" << log << '\n';
        }
        glDeleteShader(vShader);
        glDeleteShader(fShader);
    }

    void use() const { glUseProgram(id); }
    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()),
                           1, GL_FALSE, glm::value_ptr(mat));
    }
    void setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(glGetUniformLocation(id, name.c_str()),
                     1, &value[0]);
    }
};
 
// ===========================================================
// ⚡ MODEL (Oddiy Cube)
// ===========================================================
struct Model {
    unsigned int VAO, VBO;

    void load() {
        float vertices[] = {
            // Positions          // Normals
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,-1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 0.0f,-1.0f,
             0.5f,  0.5f, -0.5f,  0.0f, 0.0f,-1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,-1.0f,
            // (Yuqoridagi kabi boshqa tomonlarni ham qo'shing.)
        };
        unsigned int indices[] = {
            0,1,2, 2,3,0
        };
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    void draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
};
 
// ===========================================================
// ⚡ MAIN FUNCTION
// ===========================================================
int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW init error!\n"; return -1;
    }
    glfwSetErrorCallback(error_callback);
    GLFWwindow *window = glfwCreateWindow(1024, 768, "KrystallEngine Demo", NULL, NULL);
    if (!window) {
        std::cerr << "Window error!\n"; return -1;
    }
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW error!\n"; return -1;
    }
    glEnable(GL_DEPTH_TEST);

    std::string vertexCode = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = aNormal;
    gl_Position = projection * view * vec4(aPos, 1.0);
}
    )";
    std::string fragCode = R"(
#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0,1.0,1.0);
    vec3 ambient = 0.1 * vec3(1.0,1.0,1.0);
    FragColor = vec4(ambient + diffuse, 1.0);
}
    )";

    Shader shader(vertexCode, fragCode);
    Model cube;
    cube.load();

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = (float)glfwGetTime();
        camera.Position = glm::vec3(sin(time) * 3.0f, 1.5f, cos(time) * 3.0f);

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1024.0f/768.0f, 0.1f, 100.0f);

        shader.use();
        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec3("lightPos", glm::vec3(2.0f, 4.0f, 2.0f));
        shader.setVec3("viewPos", camera.Position);

        cube.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}