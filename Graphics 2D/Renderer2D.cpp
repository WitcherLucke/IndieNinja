#include "Renderer2D.h"
#include <iostream>

Renderer2D::Renderer2D() : shaderProgram(0), VAO(0), VBO(0), EBO(0) {}

Renderer2D::~Renderer2D() {
    Cleanup();
}

void Renderer2D::Init() {
    const char* vertexShaderSrc = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;

        out vec2 TexCoord;

        uniform vec2 position;
        uniform vec2 size;

        void main() {
            vec2 scaledPos = aPos * size + position;
            gl_Position = vec4(scaledPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        })";

    const char* fragmentShaderSrc = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D tex;
        void main() {
            FragColor = texture(tex, TexCoord);
        })";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSrc, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(fs);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);

    float vertices[] = {
        // positions  // tex coords
        0.0f, 1.0f,   0.0f, 1.0f, // Top-left
        0.0f, 0.0f,   0.0f, 0.0f, // Bottom-left
        1.0f, 0.0f,   1.0f, 0.0f, // Bottom-right
        1.0f, 1.0f,   1.0f, 1.0f  // Top-right
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void Renderer2D::DrawImage(const Image& image, float x, float y, float width, float height) {
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, image.GetTextureID());
    glUniform1i(glGetUniformLocation(shaderProgram, "tex"), 0);

    glUniform2f(glGetUniformLocation(shaderProgram, "position"), x, y);
    glUniform2f(glGetUniformLocation(shaderProgram, "size"), width, height);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Renderer2D::Cleanup() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
}