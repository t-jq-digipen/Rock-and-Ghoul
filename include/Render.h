#pragma once
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Remath.h"
class Render {
public:
    Render();
    ~Render();

    void Init();
    void DrawSquare(const glm::vec2& pos, float size, const glm::vec3& color);

    void DrawTexturedQuad(const Vec2& pos, const Vec2& scale, GLuint texture);
    void DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec3& color);
    static GLuint LoadTexture(const std::string& path);
    void DrawCircle(const glm::vec2& center, float radius, const Vec3& color, int segments);

    void DrawRect(const Vec2& pos, float width, float height, const Vec3& color);

    bool IsAreaTouched(const glm::vec2& mousePos, const glm::vec2& areaPos, float width, float height);

private:
    GLuint vao, vbo, shaderProgram;
    glm::mat4 projection;

    GLuint CompileShader(const char* source, GLenum type);
    GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
};
