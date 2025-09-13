#ifndef RENDER_H
#define RENDER_H

#include <GL/glew.h>
#include <glm/glm.hpp>


class Render {
public:
    Render();
    ~Render();

    void Init();  // setup VAO, VBO, shaders
    void DrawSquare(const glm::vec2& pos, float size, const glm::vec3& color);

private:
    GLuint vao, vbo;
    GLuint shaderProgram;
    glm::mat4 projection;

    GLuint CompileShader(const char* source, GLenum type);
    GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
};

#endif // RENDER_H
