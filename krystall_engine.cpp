// =========================================================
// KRYSTALLENGINE
// QISM 1 - ASOS / HEADER / MATH / BASE CLASSES
// =========================================================
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cassert>
#include <map>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// STB va TinyOBJ implementatsiya
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

// =========================================================
// Matematik klasslar
// =========================================================
struct Vec3 {
    float x, y, z;
};
struct Mat4 {
    float m[16];
};
struct Camera {
    float posX = 0.0f, posY = 1.5f, posZ = 3.0f;
    float yaw = -90.0f, pitch = 0.0f;
    float speed = 0.1f, sensitivity = 0.1f;

    void processInput(int key) {
        float cosY = cos(yaw * 3.1415926535f / 180.0f);
        float sinY = sin(yaw * 3.1415926535f / 180.0f);
        if (key == GLFW_KEY_W) { posX += cosY * speed; posZ += sinY * speed; }
        if (key == GLFW_KEY_S) { posX -= cosY * speed; posZ -= sinY * speed; }
        if (key == GLFW_KEY_A) { posX += sinY * speed; posZ -= cosY * speed; }
        if (key == GLFW_KEY_D) { posX -= sinY * speed; posZ += cosY * speed; }
    }
};
Camera camera;

double lastX = 400, lastY = 300;

Mat4 multiply(const Mat4 &a, const Mat4 &b) {
    Mat4 result = {};
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            result.m[i * 4 + j] = a.m[i * 4 + 0] * b.m[0 * 4 + j] +
                                   a.m[i * 4 + 1] * b.m[1 * 4 + j] +
                                   a.m[i * 4 + 2] * b.m[2 * 4 + j] +
                                   a.m[i * 4 + 3] * b.m[3 * 4 + j];
        }
    }
    return result;
}
Mat4 identity() {
    Mat4 mat = {};
    for(int i = 0; i < 16; ++i) mat.m[i] = 0.0f;
    mat.m[0] = mat.m[5] = mat.m[10] = mat.m[15] = 1.0f;
    return mat;
}
Mat4 translate(float x, float y, float z) {
    Mat4 mat = identity();
    mat.m[12] = x;
    mat.m[13] = y;
    mat.m[14] = z;
    return mat;
}
Mat4 rotateY(float angle) {
    Mat4 mat = identity();
    float c = cos(angle), s = sin(angle);
    mat.m[0] = c;
    mat.m[2] = s;
    mat.m[8] = -s;
    mat.m[10] = c;
    return mat;
}
Mat4 perspective(float fov, float aspect, float near, float far) {
    Mat4 mat = {};
    float tanHalfFOV = tan(fov / 2.0f);
    mat.m[0] = 1.0f / (aspect * tanHalfFOV);
    mat.m[5] = 1.0f / tanHalfFOV;
    mat.m[10] = - (far + near) / (far - near);
    mat.m[11] = -1.0f;
    mat.m[14] = - (2.0f * far * near) / (far - near);
    return mat;
}

// =========================================================
// CALLBACKS
// =========================================================
void error_callback(int error, const char* description) {
    std::cerr << "GLFW Xato: " << description << '\n';
}

// =========================================================
// Model Struct
// =========================================================
struct Model {
    GLuint vao, vbo, ebo;
    size_t indexCount;

    bool load(const std::string &path) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
        if (!ret) {
            std::cerr << "Model yuklashda xato: " << warn << " " << err << '\n';
            return false;
        }

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        for (const auto &shape : shapes) {
            for (const auto &index : shape.mesh.indices) {
                float vx = attrib.vertices[3 * index.vertex_index + 0];
                float vy = attrib.vertices[3 * index.vertex_index + 1];
                float vz = attrib.vertices[3 * index.vertex_index + 2];
                vertices.insert(vertices.end(), {vx, vy, vz});
                indices.push_back((unsigned int)indices.size());
            }
        }

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);

        indexCount = indices.size();
        return true;
    }

    void draw() {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }
};