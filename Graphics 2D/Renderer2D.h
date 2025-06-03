#pragma once
#include <GL/glew.h>
#include "Image.h"

class Renderer2D {
public:
    Renderer2D();
    ~Renderer2D();

    void Init();
    void Cleanup();
    void DrawImage(const Image& image, float x, float y, float width, float height); // Draw-ul trebuie sa fie functie membra a clasei image

private:
    GLuint shaderProgram, VAO, VBO, EBO;
};