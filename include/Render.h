/* ========================================================
 * @file        Render.h
 * @author      Kervin Ong Guan Cheng | kervinguancheng.ong@digipen.edu
 * @date        02 OCT 2025
 *
 * @brief   Rendering system class, consisting function declarations for drawing 2D
 *          primitives, images, and text using OpenGL.
 *
 * The Render class provides a interface for all
 * rendering operations including shapes, images, instanced
 * rendering, and text rendering. It manages OpenGL resources,
 * texture caching, and uses FreeType for font rendering.
 *
 * Functionalities include:
 * - Drawing shapes (rectangles, squares, lines, circles)
 * - Loading, caching, and unloading textures
 * - Drawing images (full, scaled, sub-regions, rotated)
 * - Instanced rendering 
 * - Text rendering with FreeType
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Remath.h"
#include <fstream>
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H

 /**
  * @struct Image
  * @brief Represents a loaded OpenGL texture with its dimensions in pixels.
  *
  * The Image struct encapsulates a texture ID and its associated
  * width/height, and provides a helper to check if the texture is valid.
  */
struct Image {
    GLuint textureID;
    int widthPixels;
    int heightPixels;

    Image() : textureID(0), widthPixels{ 0 }, heightPixels{ 0 } {} //default constructer

    /**
     * @brief Constructs an Image from an existing OpenGL texture.
     * @param texID The OpenGL texture ID.
     * @param widthpx The width of the image in pixels.
     * @param heightpx The height of the image in pixels.
     */
    Image(GLuint const& texID, int const& widthpx, int const& heightpx) : textureID(texID), widthPixels(widthpx), heightPixels(heightpx) {}

    /**
     * @brief Checks if the Image holds a valid OpenGL texture.
     * @return True if valid (textureID != 0), false otherwise.
     */
    bool IsValid() const { return textureID != 0; }
};


/**
 * @brief Creates an Image by loading a texture from a file path.
 * @param path The path to the image file.
 * @return An Image containing the texture ID and its dimensions.
 */
Image CreateImage(const std::string& path);




/**
 * @struct Character
 * @brief Stores glyph rendering information for a single character.
 *
 * Used in text rendering via FreeType. Each Character struct
 * represents one glyph's texture and its layout stuff.
 */
struct Character {
    unsigned int TextureID;  // OpenGL texture ID for the glyph
    glm::ivec2   Size;  // Glyph width and height in pixels
    glm::ivec2   Bearing; // Offset from baseline to left/top of glyph
    unsigned int Advance; // Offset in pixels to advance to next glyph
};



/**
 * @struct InstanceData
 * @brief Per-instance data for instanced image rendering.
 *
 * Each InstanceData struct describes a single sprite's
 * position, scale, rotation, and texture UV region.
 */
struct InstanceData {
    Vec2 position; //World position of the instance
    Vec2 scale; //Scale factors (x, y) for the instance
    glm::vec4 uvRect;  // u0, v0, u1, v1
    float rotation; //in radians
};




class Render {
public:

    /**
     * @brief Retrieves the instance of the Render class.
     * @return Reference to the single Render instance.
     */
    static Render& GetInstance() {
        static Render instance;
        return instance;
    }

    ~Render(); //destructor 

    /**
    * @brief Initializes the Render system, setting up shaders, buffers,
    projection matrices, and FreeType resources.
    */
    void Init();

    /**
    * @brief Releases all allocated GPU and CPU resources used by the
    Render system, including textures, buffers, and FreeType data.
    */
    void Shutdown();

    /**
     * @brief Clears the screen with the specified background color.
     * @param color The RGBA color to fill the screen with.
    */
    void ClearBackground(const Colour4& color); //use this to clear background, then set your background color



    //Draw actions
    /**
     * @brief Draws a colored square at the given position.
     * @param pos The 2D position (x, y) of the square.
     * @param size The width and height of the square.
     * @param color The RGB color of the square.
     * @param rotation Rotation angle in radians (default = 0.0f).
    */
    void DrawSquare(const glm::vec2& pos, float size, const glm::vec3& color, float rotation = 0.0f); 


    /**
    * @brief Draws a colored rectangle at the given position.
    * @param pos The top-left position of the rectangle.
    * @param width Width of the rectangle in pixels.
    * @param height Height of the rectangle in pixels.
    * @param color The RGB color of the rectangle.
    * @param rotation Rotation angle in radians (default = 0.0f).
    */
    void DrawRect(const Vec2& pos, float width, float height, const Colour3& color, float rotation = 0.0f);


    /**
     * @brief Draws a textured quad at the specified position and scale.
     * @param pos The 2D position of the quad's origin.
     * @param scale The width and height of the quad.
     * @param texture The OpenGL texture ID to apply to the quad.
     * @param rotation Rotation angle in radians to apply (default = 0.0f, no rotation).
     */
    void DrawTexturedQuad(const Vec2& pos, const Vec2& scale, GLuint texture, float rotation = 0.0f);


    /**
     * @brief Draws a colored line between two points.
     * @param start The starting point of the line (x, y).
     * @param end The ending point of the line (x, y).
     * @param color The RGB color of the line.
     */
    void DrawLine(const Vec2& start, const Vec2& end, const Colour3& color);


    /**
    * @brief Draws a circle at the specified center position.
    * @param center The center position of the circle.
    * @param radius The radius of the circle.
    * @param color The RGB color of the circle.
    * @param segments Number of line segments to approximate the circle.
    * @param rotation Rotation applied to the circle (default = 0.0f).
    */
    void DrawCircle(const Vec2& center, float radius, const Colour3& color, int segments,float rotation = 0.0f);


    //Image/Texture
    /**
    * @brief Draws an image at the specified (x, y) position.
    * @param image The Image object containing texture data.
    * @param x The x position of the image.
    * @param y The y position of the image.
    */
    void DrawImage(const Image& image, float x, float y);


    /**
    * @brief Draws an image with specific width, height, and rotation.
    * @param image The Image object containing texture data.
    * @param x The x position of the image.
    * @param y The y position of the image.
    * @param w The desired width of the drawn image.
    * @param h The desired height of the drawn image.
    * @param rotation Rotation angle in radians (default = 0.0f).
    */
    void Draw_SpecImage(const Image& image, float x, float y, float w, float h, float rotation = 0.0f);


    /**
     * @brief Draws a cropped sub-region of an image.
     * @param image The Image object containing texture data.
     * @param x, y The position to draw the cropped image.
     * @param w, h The width and height to draw.
     * @param u0, v0 Top-left corner of the sub-region in pixels.
     * @param u1, v1 Bottom-right corner of the sub-region in pixels.
     * @param rotation Rotation angle in radians (default = 0.0f).
     */
    void DrawImageSub(const Image& image, float x, float y, float w, float h, float u0, float v0, float u1, float v1, float rotation = 0.0f);


    /**
     * @brief Draws an image at the specified position, scaled by the given factors.
     * @param image The Image object containing texture data.
     * @param x The x position of the image's origin.
     * @param y The y position of the image's origin.
     * @param scaleX The horizontal scale factor to apply.
     * @param scaleY The vertical scale factor to apply.
     * @param rotation Rotation angle in radians to apply to the image.
     */
    void DrawImageScaled(const Image& image, float x, float y, float scaleX, float scaleY, float rotation); 


    /**
     * @brief Draws an image at the specified position with rotation applied.
     * @param image The Image object containing texture data.
     * @param x The x position of the image's origin.
     * @param y The y position of the image's origin.
     * @param angle The rotation angle in radians.
     */
    void DrawImageRotated(const Image& image, float x, float y, float angle);


    /**
     * @brief Retrieves the width of the given image in pixels.
     * @param image The Image object containing texture data.
     * @return The width of the image in pixels.
     */
    int GetImageWidth(const Image& image) const;


    /**
     * @brief Retrieves the height of the given image in pixels.
     * @param image The Image object containing texture data.
     * @return The height of the image in pixels.
     */
    int GetImageHeight(const Image& image) const;              



    //Texture
    /**
    * @brief Loads a texture from a file and caches it for reuse.
    * @param path File path to the texture.
    * @return OpenGL texture ID for the loaded texture.
    */
    GLuint LoadTexture(const std::string& path);


    /**
    * @brief Unloads a texture by its file path, removing it from cache.
    * @param path Path of the texture to unload.
    */
    void UnloadTexture(const std::string& path);


    /**
    * @brief Unloads a texture by its OpenGL texture ID.
    * @param textureID The texture ID to unload.
    */
    void UnloadTexture(GLuint textureID);


    /**
    * @brief Clears all cached textures from GPU memory.
    */
    void ClearAllTextures();
    
    // amended helper functions for json bc they load the wrong texture for rock
    /**
    * @brief Associates a texture ID with its file path in the path cache.
    * @param texID The OpenGL texture ID to register.
    * @param path The file path corresponding to the texture.
    */
    void RegisterTexturePath(GLuint texID, const std::string& path);


    /**
     * @brief Retrieves the file path associated with a given texture ID.
     * @param texID The OpenGL texture ID to get.
     * @return The file path as a string. Returns an empty string if not found.
     */
    std::string GetTexturePath(GLuint texID) const;


    /**
     * @brief Alternative accessor for retrieving the file path of a texture.
     * @param imgID The OpenGL texture ID to get.
     * @return The file path as a string. Returns an empty string if not found.
     */
    std::string Get_TexturePath(const GLuint& imgID);



    //Draw text
    /**
    * @brief Initializes FreeType resources for text rendering.
    */
    void InitTextRendering();


    /**
    * @brief Draws text on screen using a loaded FreeType font.
    * @param text The string to render.
    * @param x The x position of the text baseline.
    * @param y The y position of the text baseline.
    * @param scale Scale factor for text size.
    * @param color The RGB color of the text.
    */
    void DrawText(std::string text, float x, float y, float scale, const Colour3& color);



    //instance render
    /**
    * @brief Draws multiple instances of an image efficiently using instanced rendering.
    * @param image The image to draw.
    * @param instances A list of instance data (position, scale, UV, rotation).
    */
    void DrawImageInstanced(const Image& image, const std::vector<InstanceData>& instances);

    void DrawRectInstanced(const std::vector<InstanceData>& instances, const Colour3& color);


    /**
     * @brief Checks if a given mouse position lies within a rectangular area.
     * @param mousePos The mouse position in screen coordinates.
     * @param areaPos The top-left position of the rectangular area.
     * @param width The width of the rectangular area.
     * @param height The height of the rectangular area.
     * @return True if the mouse position is inside the area, false otherwise.
     */
    bool IsAreaTouched(const glm::vec2& mousePos, const glm::vec2& areaPos, float width, float height);


private:
    /**
     * @brief Pointer to the instance of the Render class.
     */
    static Render* instance;

    /**
     * @brief Default constructor for the Render system.
     *        Initializes internal members and sets up default OpenGL state.
     */
    Render();


    /**
     * @brief Deleted copy constructor
     */
    Render(const Render&) = delete;


    /**
     * @brief Deleted assignment operator
     */
    Render& operator=(const Render&) = delete;

    //vao/vbo and 4x4 projection matrix used for transforming screen-space coordinates
    GLuint vao, vbo, shaderProgram;
    GLuint InstanceVBO, InstanceShaderProgram;
    GLuint textVAO, textVBO;
    glm::mat4 projection;


    //compile,link,validate shaders into program from file
    /**
    * @brief Reads the contents of a text file into a string.
    * @param path The file path to read from.
    * @return The file contents as a string.
    * @throws std::runtime_error if the file cannot be opened.
    */
    std::string readTextFile(const std::string& path);


    /**
     * @brief Compiles an OpenGL shader from a given file.
     * @param path The path to the shader source file.
     * @param type The type of shader to compile (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, etc.).
     * @return The OpenGL shader object ID.
     */
    GLuint CompileShaderFromFile(const std::string& path, GLenum type);


    /**
     * @brief Creates an OpenGL shader program by compiling, linking,
     *        and validating vertex and fragment shaders from files.
     * @param vertPath File path to the vertex shader source.
     * @param fragPath File path to the fragment shader source.
     * @return The OpenGL shader program object ID.
     */
    GLuint CreateProgramFromFiles(const std::string& vertPath, const std::string& fragPath);

    //texture and path cache
    std::unordered_map<std::string, GLuint> textureCache; //Cache of loaded textures mapped by file path
    std::unordered_map<GLuint, std::string> pathCache; //Reverse lookup cache mapping texture IDs to file paths.
    std::map<char, Character> Characters; //Character map storing FreeType glyph data for text rendering

    // to load for json
    std::unordered_map<GLuint, std::string> texturePathMap;

    

};
