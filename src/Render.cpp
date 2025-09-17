#include "Render.h"
#include "glapp.h"
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// stb_image
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Vertex + Fragment shaders
const char* vertexSrc = R"(
#version 450 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;

uniform mat4 uProjection;
uniform vec2 uPos;
uniform vec2 uScale;

out vec2 TexCoord;

void main() {
    vec2 scaled = aPos * uScale + uPos;
    gl_Position = uProjection * vec4(scaled, 0.0, 1.0);
    TexCoord = aTex;
}
)";

const char* fragmentSrc = R"(
#version 450 core
out vec4 FragColor;

uniform vec3 uColor;
uniform sampler2D uTexture;
uniform bool uUseTexture;

in vec2 TexCoord;

void main() {
    if (uUseTexture)
        FragColor = texture(uTexture, TexCoord);
    else
        FragColor = vec4(uColor, 1.0);
}
)";


Render::Render() : vao(0), vbo(0), shaderProgram(0) {}
Render::~Render() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (shaderProgram) glDeleteProgram(shaderProgram);
}

void Render::Init() {
    // Square vertices (pos + uv)
    float vertices[] = {
        //   x,    y,   u,  v
        -0.5f, -0.5f, 0.0f, 0.0f,
         0.5f, -0.5f, 1.0f, 0.0f,
         0.5f,  0.5f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 1.0f
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attrib
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Texcoord attrib
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    // Shaders
    GLuint vsh = CompileShader(vertexSrc, GL_VERTEX_SHADER);
    GLuint fsh = CompileShader(fragmentSrc, GL_FRAGMENT_SHADER);
    shaderProgram = LinkProgram(vsh, fsh);

    glDeleteShader(vsh);
    glDeleteShader(fsh);

    // Setup projection
    projection = glm::ortho(
        0.0f, static_cast<float>(GLApp::width),
        0.0f, static_cast<float>(GLApp::height),
        -1.0f, 1.0f
    );
}

void Render::DrawSquare(const glm::vec2& pos, float size, const glm::vec3& color) {
    glUseProgram(shaderProgram);

    // upload projection
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projection[0][0]);

    // upload pos, size (use size for both X and Y), color
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), pos.x, pos.y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), size, size);
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), color.r, color.g, color.b);

    // no texture
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), GL_FALSE);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    glUseProgram(0);
}

void Render::DrawTexturedQuad(const glm::vec2& pos, const glm::vec2& scale, GLuint texture) {
    glUseProgram(shaderProgram);

    // upload projection
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projection[0][0]);

    // upload pos + scale
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), pos.x, pos.y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), scale.x, scale.y);

    // enable texture
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), GL_TRUE);

    // bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

    // draw
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    glUseProgram(0);
}

GLuint Render::LoadTexture(const std::string& path) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return 0;
    }

    GLenum format = GL_RGB;
    if (channels == 1) format = GL_RED;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 4) format = GL_RGBA;

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
        format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // set parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return textureID;
}

GLuint Render::CompileShader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compile error: " << infoLog << std::endl;
    }
    return shader;
}

GLuint Render::LinkProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader link error: " << infoLog << std::endl;
    }
    return program;
}
