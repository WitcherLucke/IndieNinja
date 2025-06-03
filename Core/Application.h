#pragma once

#include <GL/glew.h>
#include "./glfw3.h"
#include "./Graphics 2D/Renderer2D.h"
#include "./Graphics 2D/Image.h"

class Application { // rename in GLContext eventual
public:
    Application();
    ~Application();

    bool Init();
    void Run();

private:
    GLFWwindow* window;
    Image image; // TODO: asta scos de-aici
    Renderer2D renderer;
};