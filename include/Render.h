#pragma once
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Remath.h"
#include <fstream>



class Render {
public:
    static Render& GetInstance() {
        if (!instance) {
            instance = new Render();
        }
        return *instance;
    }

    static void DestroyInstance() {
        delete instance;
        instance = nullptr;
    }

    ~Render();

    void Init();

    void ClearBackground(const Colour4& color); //use this to clear background, then set your background color

    //Draw actions
    void DrawSquare(const glm::vec2& pos, float size, const glm::vec3& color); //draw square/point depending on size
    void DrawRect(const Vec2& pos, float width, float height, const Colour3& color);
    void DrawTexturedQuad(const Vec2& pos, const Vec2& scale, GLuint texture);
    void DrawLine(const Vec2& start, const Vec2& end, const Colour3& color);
    void DrawCircle(const glm::vec2& center, float radius, const Colour3& color, int segments);


    //Image/Texture actions (texture caching maybe)
    GLuint LoadTexture(const std::string& path);
    void UnloadTexture(const std::string& path); //if want use file path instead
    void UnloadTexture(GLuint textureID); //to follow current 
    void ClearAllTextures(); //can use this in the unload of each state etc.mainmenu unload, level1 unload
    

    

    bool IsAreaTouched(const glm::vec2& mousePos, const glm::vec2& areaPos, float width, float height);

private:
    static Render* instance;
    Render();
    GLuint vao, vbo, shaderProgram;
    glm::mat4 projection;


    //compile,link,validate shaders into program from file
    std::string readTextFile(const std::string& path);
    GLuint CompileShaderFromFile(const std::string& path, GLenum type);
    GLuint CreateProgramFromFiles(const std::string& vertPath, const std::string& fragPath);

    //texture cache (allow multiple to share common sprite)
    std::unordered_map<std::string, GLuint> textureCache;

};
