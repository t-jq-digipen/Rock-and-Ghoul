#pragma once
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H


//struct for each individual text
struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};

//inline std::map<char, Character> Characters;



class Render {
public:
    Render();
    ~Render();

    void Init();

    void InitTextRendering(); //helper init function for text rendering

    void DrawSquare(const glm::vec2& pos, float size, const glm::vec3& color);

    void DrawSquare(const glm::mat4& model, const glm::vec3& color); //overload version to handle rotation/scale

    void DrawTexturedQuad(const glm::vec2& pos, const glm::vec2& scale, GLuint texture);

    void DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec3& color);

    static GLuint LoadTexture(const std::string& path);

    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color); //render text function

private:
    GLuint vao, vbo, shaderProgram;
    glm::mat4 projection;

    //for the text rendering 
    GLuint textVAO, textVBO;
    GLuint textShaderProgram;
    std::map<char, Character> Characters;

    GLuint CompileShader(const char* source, GLenum type);
    GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
};
