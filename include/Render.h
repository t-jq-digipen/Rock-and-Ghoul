#pragma once
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

enum class BgFit { //background sizes
    Stretch, 
    Contain, 
    Cover 
};

class Render {
public:
    Render();
    ~Render();

    void Init();
    void DrawSquare(const glm::vec2& pos, float size, const glm::vec3& color);

    void DrawSquare(const glm::mat4& model, const glm::vec3& color); //overload version to handle rotation/scale

    void DrawTexturedQuad(const glm::vec2& pos, const glm::vec2& scale, GLuint texture);

    void DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec3& color);

    void DrawBackground(GLuint tex, int texW, int texH, BgFit fit = BgFit::Stretch); //reusing drawTexturedQuad

    static GLuint LoadTexture(const std::string& path);

private:
    GLuint vao, vbo, shaderProgram;
    glm::mat4 projection;

    GLuint CompileShader(const char* source, GLenum type);
    GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
};
