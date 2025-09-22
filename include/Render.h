#pragma once
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

class Render {
public:
    Render();
    ~Render();

    void Init();
    void DrawSquare(const glm::vec2& pos, float size, const glm::vec3& color);

    void DrawTexturedQuad(const glm::vec2& pos, const glm::vec2& scale, GLuint texture);

    void DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec3& color);
    static GLuint LoadTexture(const std::string& path);
    void DrawCircle(const glm::vec2& center, float radius, const glm::vec3& color, int segments);

private:
    GLuint vao, vbo, shaderProgram;
    glm::mat4 projection;

    GLuint CompileShader(const char* source, GLenum type);
    GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
};
