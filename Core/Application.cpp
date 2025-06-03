#include "Application.h"
#include <iostream>

Application::Application() : window(nullptr) {}

Application::~Application() {
    glfwTerminate();
}

bool Application::Init() {
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 600, "2D Renderer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Window creation failed\n";
        return false;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW init failed\n";
        return false;
    }

    renderer.Init();
    //return image.LoadFromDiskStandard("sprite.png"); // de scos de-aici
    return image.LoadFromDiskBC7("sprite.dds");
}

void Application::Run() {
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        renderer.DrawImage(image, -0.5f, -0.5f, 1.0f, 1.0f);  // Normalized coordinates

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    renderer.Cleanup();
}