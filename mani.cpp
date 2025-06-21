#include <iostream>
#include "krystall_engine.h" // Model, Renderer, Node, Light va boshqalar e'lon qilingan header
#include <GL/glew.h>
#include <GLFW/glfw3.h>

int main() {
    // ⚡️ 1. GLFW init
    if (!glfwInit()) {
        std::cerr << "GLFW init xato!\n";
        return -1;
    }

    // ⚡️ 2. Oyna sozlash
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow *window = glfwCreateWindow(1024, 768, "KrystallEngine Demo", nullptr, nullptr);
    if (!window) {
        std::cerr << "Oyna yaratilmadi!\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // ⚡️ 3. GLEW init
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW init xato!\n";
        return -1;
    }

    // ⚡️ 4. Renderer va Model
    Renderer renderer;
    if (!renderer.init("shaders/basic.vert", "shaders/basic.frag")) {
        std::cerr << "Shader yuklashda xato!\n";
        return -1;
    }

    Model cube;
    if (!cube.load("models/cube.obj")) {
        std::cerr << "Cube modelini yuklashda xato!\n";
        return -1;
    }

    // ⚡️ 5. Node va Animatsiya
    Node root{cube, identity()};
    Node child{cube, translate(1.5f, 0.0f, 0.0f)};
    root.addChild(child);

    Light light{{2.0f, 4.0f, 2.0f}, {1.0f, 1.0f, 1.0f}};
    Animation walkAnimation{"Walk", 2.5f};
    walkAnimation.play();

    // ⚡️ 6. Asosiy loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Mat4 view = identity();
        Mat4 proj = perspective(3.1415926535f / 3.0f, 1024.0f / 768.0f, 0.1f, 100.0f);

        root.draw(renderer, view, proj, light);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ⚡️ 7. Tozalash
    glfwTerminate();
    return 0;
}