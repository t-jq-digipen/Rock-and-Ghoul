#pragma once
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Remath.h"
#include <fstream>

// Image structure to hold texture data 
struct Image {
    GLuint textureID;
    int width;
    int height;
    std::string path;

    Image() : textureID(0), width(0), height(0) {}
    Image(GLuint id, int w, int h, const std::string& p)
        : textureID(id), width(w), height(h), path(p) {
    }

    bool IsValid() const { return textureID != 0; }
};

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
    void DrawTexturedQuad(const Vec2& pos, const Vec2& scale, GLuint texture, float rotation = 0.0f); //added additional rotation (no rotate by defauklt)
    void DrawLine(const Vec2& start, const Vec2& end, const Colour3& color);
    void DrawCircle(const glm::vec2& center, float radius, const Colour3& color, int segments);


    //Image/Texture
    Image LoadImage(const std::string& path);                    // something like CP_Image_Load
    void DrawImage(const Image& image, float x, float y);        // something like CP_Image_Draw
    void Draw_SpecImage(const Image& image, float x, float y, float w, float h); // draw image to given width, height

    //void DrawImageSub(const Image& image, float x, float y, float w, float h, float u0, float v0, float u1, float v1); // <-- im stuck 
    //void DrawTexturedQuadUV(const Vec2& pos, const Vec2& scale, GLuint texture, float u0, float v0, float u1, float v2, int alpha = 255, float rotation = 0.0f); //helper for draw_subImage
    //void DrawImageAdvanced(const Image& image, float x, float y, float w, float h, float u, float v, float uWidth, float vHeight);
   
    void DrawImageScaled(const Image& image, float x, float y, float scaleX, float scaleY); 
    void DrawImageRotated(const Image& image, float x, float y, float angle);
    int GetImageWidth(const Image& image) const;                 // get the image width
    int GetImageHeight(const Image& image) const;                // get the height
    void UnloadImage(Image& image);
    void ClearAllImages();


    //Image/Texture actions (texture caching maybe)
    GLuint LoadTexture(const std::string& path);
    void UnloadTexture(const std::string& path); //if want use file path instead
    void UnloadTexture(GLuint textureID); //to follow current 
    void ClearAllTextures(); //can use this in the unload of each state etc.mainmenu unload, level1 unload
    
    //draw_font maybe later

    

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
    std::unordered_map<std::string, Image> imageCache;

    Image CreateImageFromTexture(GLuint textureID, int width, int height, const std::string& path);
    

};
