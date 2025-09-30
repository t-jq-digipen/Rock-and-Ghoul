#pragma once
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Remath.h"
#include <fstream>
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H

// Image structure to hold texture data 
struct Image {
    GLuint textureID;
    int widthPixels;
    int heightPixels;

    Image() : textureID(0), widthPixels{ 0 }, heightPixels{ 0 } {}
    Image(GLuint const& texID, int const& widthpx, int const& heightpx) : textureID(texID), widthPixels(widthpx), heightPixels(heightpx) {}

    
    bool IsValid() const { return textureID != 0; }
};

Image CreateImage(const std::string& path);

//struct for each individual text
struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};

class Render {
private:

    /*Render();

    Render(const Render&)               = delete;
    Render& operator=(const Render&)    = delete;

    ~Render();*/
public:
    static Render& GetInstance() {
        if (!instance) {
            instance = new Render();
        }
        return *instance;
    }

    /*static Render& GetInstance() {
        static Render instance;
        return instance;
    }*/

    static void DestroyInstance() {
        delete instance;
        instance = nullptr;
    }

    ~Render();

    void Init();

    void ClearBackground(const Colour4& color); //use this to clear background, then set your background color

    //Draw actions
    void DrawSquare(const glm::vec2& pos, float size, const glm::vec3& color, float rotation = 0.0f); //draw square/point/hotspots depending on size
    void DrawRect(const Vec2& pos, float width, float height, const Colour3& color, float rotation = 0.0f);
    void DrawTexturedQuad(const Vec2& pos, const Vec2& scale, GLuint texture, float rotation = 0.0f); //added additional rotation (no rotate by defauklt)
    void DrawLine(const Vec2& start, const Vec2& end, const Colour3& color);
    void DrawCircle(const Vec2& center, float radius, const Colour3& color, int segments,float rotation = 0.0f);


    //Image/Texture
    // Image LoadImage(const std::string& path);                    // something like CP_Image_Load
    void DrawImage(const Image& image, float x, float y);        // something like CP_Image_Draw
    void Draw_SpecImage(const Image& image, float x, float y, float w, float h); // draw image to given width, height

    //usage for drawImage Sub
    //- x, y: the position of where u want the cropped image to be
    //- w, h: how long or high u want the cropped image to be
    //- u, v portion of the texture/image u wanna to display
    // etc u0, u1 (0,256) means span 256 pixels from top left to right
    // v0, v1 (0, 256) means span 256 pixels from top left to down
    void DrawImageSub(const Image& image, float x, float y, float w, float h, float u0, float v0, float u1, float v1);

   
    void DrawImageScaled(const Image& image, float x, float y, float scaleX, float scaleY); 
    void DrawImageRotated(const Image& image, float x, float y, float angle); //note that angles u input must be in radians
    int GetImageWidth(const Image& image) const;                 // get the image width
    int GetImageHeight(const Image& image) const;                // get the height
    /*void UnloadImage(Image& image);
    void ClearAllImages();*/


    //Image/Texture actions (texture caching maybe)
    GLuint LoadTexture(const std::string& path);
    void UnloadTexture(const std::string& path); //if want use file path instead
    void UnloadTexture(GLuint textureID); //to follow current 
    void ClearAllTextures(); //can use this in the unload of each state etc.mainmenu unload, level1 unload
    
    //draw_font maybe later
    void InitTextRendering();
    void DrawText(std::string text, float x, float y, float scale, const Colour3& color); //use: text, position, size, color
    //maybe set_font 
    

    bool IsAreaTouched(const glm::vec2& mousePos, const glm::vec2& areaPos, float width, float height);


private:
    static Render* instance;
    Render();
    GLuint vao, vbo, shaderProgram;
    GLuint textVAO, textVBO; //text vaovbos
    glm::mat4 projection;


    //compile,link,validate shaders into program from file
    std::string readTextFile(const std::string& path);
    GLuint CompileShaderFromFile(const std::string& path, GLenum type);
    GLuint CreateProgramFromFiles(const std::string& vertPath, const std::string& fragPath);

    //texture cache (allow multiple to share common sprite)
    std::unordered_map<std::string, GLuint> textureCache;
    // std::unordered_map<std::string, Image> imageCache;
    std::unordered_map<GLuint, std::string> pathCache;
    std::map<char, Character> Characters;

    // Image CreateImageFromTexture(GLuint textureID, int width, int height, const std::string& path);
    

};
