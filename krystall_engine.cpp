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
// =========================================================
// KRYSTALLENGINE
// QISM 2 - SHADERLAR / UTILS
// =========================================================
#include <string>
#include <fstream>
#include <sstream>

struct Shader {
    GLuint id;

    static GLuint compileShader(const std::string &source, GLenum type) {
        GLuint shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char info[1024];
            glGetShaderInfoLog(shader, sizeof(info), nullptr, info);
            std::cerr << "[Shader xato] " << info << '\n';
        }
        return shader;
    }

    static Shader fromFiles(const std::string &vertPath, const std::string &fragPath) {
        std::ifstream vFile(vertPath), fFile(fragPath);
        std::stringstream vStream, fStream;
        vStream << vFile.rdbuf();
        fStream << fFile.rdbuf();

        GLuint v = compileShader(vStream.str(), GL_VERTEX_SHADER);
        GLuint f = compileShader(fStream.str(), GL_FRAGMENT_SHADER);

        Shader shader;
        shader.id = glCreateProgram();
        glAttachShader(shader.id, v);
        glAttachShader(shader.id, f);
        glLinkProgram(shader.id);

        GLint success;
        glGetProgramiv(shader.id, GL_LINK_STATUS, &success);
        if (!success) {
            char info[1024];
            glGetProgramInfoLog(shader.id, sizeof(info), nullptr, info);
            std::cerr << "[Shader link xato] " << info << '\n';
        }
        glDeleteShader(v);
        glDeleteShader(f);

        return shader;
    }

    void use() {
        glUseProgram(id);
    }

    void setMat4(const std::string &name, const Mat4 &mat) {
        glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()),
                           1, GL_FALSE, mat.m);
    }
    void setVec3(const std::string &name, float x, float y, float z) {
        glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
    }
    void setInt(const std::string &name, int value) {
        glUniform1i(glGetUniformLocation(id, name.c_str()), value);
    }
};

struct Light {
    Vec3 position;
    Vec3 color;
};

struct Renderer {
    Shader basicShader;

    bool init(const std::string &vertPath, const std::string &fragPath) {
        basicShader = Shader::fromFiles(vertPath, fragPath);
        return basicShader.id != 0;
    }

    void draw(const Model &model, const Mat4 &modelMat, const Mat4 &view, const Mat4 &proj, const Light &light) {
        basicShader.use();
        basicShader.setMat4("model", modelMat);
        basicShader.setMat4("view", view);
        basicShader.setMat4("projection", proj);
        basicShader.setVec3("lightColor", light.color.x, light.color.y, light.color.z);
        model.draw();
    }
};
// =========================================================
// KRYSTALLENGINE
// QISM 3 - SCENE / ANIMATION / MAIN LOOP
// =========================================================
#include <vector>
#include <string>
#include <iostream>

struct Animation {
    std::string name;
    float duration;

    void play() {
        std::cout << "[Animation] Playing " << name << " (" << duration << "s)" << '\n';
    }
};

struct Node {
    Model model;
    Mat4 transform;
    std::vector<Node> children;

    void draw(Renderer &renderer, const Mat4 &view, const Mat4 &proj, const Light &light) {
        renderer.draw(model, transform, view, proj, light);
        for (auto &child : children) {
            child.draw(renderer, view, proj, light);
        }
    }

    void addChild(const Node &node) {
        children.push_back(node);
    }
};

struct Physics {
    static bool isColliding(const Node &a, const Node &b, float distance) {
        // Dummy bounding sphere collision detection
        float dx = a.transform.m[12] - b.transform.m[12];
        float dy = a.transform.m[13] - b.transform.m[13];
        float dz = a.transform.m[14] - b.transform.m[14];
        float dist = sqrt(dx * dx + dy * dy + dz * dz);
        return dist <= distance;
    }
};

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW init error!\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow *window = glfwCreateWindow(1024, 768, "KrystallEngine Demo", nullptr, nullptr);
    if (!window) {
        std::cerr << "Window yaratib bo'lmadi!\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW init error!\n";
        return -1;
    }

    Renderer renderer;
    if (!renderer.init("shaders/basic.vert", "shaders/basic.frag")) {
        std::cerr << "Shaderlarni yuklashda xato!\n";
        return -1;
    }

    Model cube;
    if (!cube.load("models/cube.obj")) {
        std::cerr << "Cube modelini yuklashda xato!\n";
        return -1;
    }

    Node root{cube, identity()};
    Node child{cube, translate(1.5f, 0.0f, 0.0f)};
    root.addChild(child);

    Light light{{2.0f, 4.0f, 2.0f}, {1.0f, 1.0f, 1.0f}};
    Animation walkAnimation{"Walk", 2.5f};
    walkAnimation.play();

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Mat4 view = identity();
        Mat4 proj = perspective(3.1415926535f / 3.0f, 1024.0f / 768.0f, 0.1f, 100.0f);
        root.draw(renderer, view, proj, light);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}