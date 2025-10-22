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
#include <sstream>
#include <nlohmann/json.hpp>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

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
    void DrawCircle(const Vec2& center, float radius, const Colour3& color, int segments);


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

    /**
    * @brief Draws text on screen bounded by a textbox using a loaded FreeType font.
    * @param text The string to render.
    * @param x The x position of the text baseline.
    * @param y The y position of the text baseline.
    * @param rowWidth The width for the textbox
    * @param scale Scale factor for text size.
    * @param color The RGB color of the text.
    */
    void DrawTextBox(const std::string& text, float x, float y, float rowWidth, float scale, const Colour3& color); //for dialogue



    //instance render
    /**
    * @brief Draws multiple instances of an image efficiently using instanced rendering.
    * @param image The image to draw.
    * @param instances A list of instance data (position, scale, UV, rotation).
    */
    void DrawImageInstanced(const Image& image, const std::vector<InstanceData>& instances);

    /**
    * @brief Draws multiple instances of a solid-colored rectangle efficiently using instanced rendering.
    *
    * This function batches all instances of rectangles into a single GPU draw call using
    * hardware instancing. Each instance uses its own position, scale, and rotation from
    * the provided instance data, but all instances share the same color.
    *
    * @param instances A list of instance data (position, scale, rotation, UV rect).
    *                  The UV rect field is ignored for solid-colored rectangles.
    * @param color     The RGB color to apply to all rendered rectangle instances.
    */
    void DrawRectInstanced(const std::vector<InstanceData>& instances, const Colour3& color);


    /**
     * @brief Checks if a given mouse position lies within a rectangular area.
     * @param mousePos The mouse position in screen coordinates.
     * @param areaPos The top-left position of the rectangular area.
     * @param width The width of the rectangular area.
     * @param height The height of the rectangular area.
     * @return True if the mouse position is inside the area, false otherwise.
     */
    bool IsAreaTouched(const Vec2& mousePos, const Vec2& areaPos, float width, float height);



    //Loading multiple textures from json file
    /**
    * @brief Loads image assets defined in a JSON file and returns their texture handles.
    *
    * Example JSON:
    * {
    *   "images": [
    *     { "path": "Assets/title.png", "alias": "title" },
    *     { "path": "Assets/play.png", "alias": "play" }
    *   ]
    * }
    *
    * @param jsonPath Path to JSON file containing image paths or objects.
    * @return std::unordered_map<std::string, GLuint> Mapping of alias (key) to texture ID (value).
    */
    std::unordered_map<std::string, GLuint> LoadImageAssetsFromJSON(const std::string& jsonPath);





    //Shader interface functions for future
    /**
    * @brief Activates a specific shader program for subsequent rendering.
    *
    * This function binds the given shader program using glUseProgram().
    * Once applied, all draw calls will use this shader until another shader
    * is bound or ResetShader() is called.
    *
    * @param shaderID OpenGL handle of the shader program to activate.
    */
    void ApplyShader(GLuint shaderID);

    /**
    * @brief Deactivates the currently bound shader program.
    *
    * Unbinds any active shader by calling glUseProgram(0), effectively
    * resetting the rendering pipeline to have no shader bound.
    */
    void ResetShader();

    /**
    * @brief Sets a floating-point uniform variable in the specified shader.
    *
    * Retrieves the uniform location within the shader program and uploads
    * the given float value using glUniform1f(). This is typically used for
    * scalar parameters such as intensity, time, or alpha.
    *
    * @param shaderID OpenGL handle of the shader program containing the uniform.
    * @param name Name of the uniform variable in the shader source. (like this name must exists in the vertexSrc/FragmentSrc)
    * @param value Floating-point value to assign to the uniform.
    */
    void SetUniform1f(GLuint ShaderID, const std::string& name, float value);

    /**
     * @brief Sets a vec2 (two-component float vector) uniform variable in a shader program.
     *
     * This function uploads a 2D vector (e.g., position, scale, or UV offset)
     * to a specified uniform variable inside the given shader program.
     * If the uniform name is not found in the shader, the function safely does nothing.
     *
     * @param shaderID   The OpenGL shader program handle (GLuint) to modify.
     * @param name       The name of the uniform variable in the shader (as declared in GLSL).
     * @param value      The glm::vec2 value to upload to the shader.
     *
     * @note This is a convenience wrapper for glUniform2f().
     *       It must be called after the shader program has been activated with ApplyShader()/glUseProgram().
     */
    void SetUniform2f(GLuint shaderID, const std::string& name, const glm::vec2& value);

    /**
    * @brief Sets a 3-component vector uniform variable in the specified shader.
    *
    * Retrieves the uniform location within the shader program and uploads
    * the given vector using glUniform3f(). Commonly used for color or direction values.
    *
    * @param shaderID OpenGL handle of the shader program containing the uniform.
    * @param name Name of the uniform variable in the shader source.
    * @param vec 3-component vector value to assign to the uniform.
    */
    void SetUniform3f(GLuint ShaderID, const std::string& name, const glm::vec3& vec);


    /**
    * @brief Sets a 4x4 matrix uniform variable in the specified shader.
    *
    * Retrieves the uniform location within the shader program and uploads
    * the given matrix using glUniformMatrix4fv(). This is typically used for
    * transformation matrices such as model, view, or projection.
    *
    * @param shaderID OpenGL handle of the shader program containing the uniform.
    * @param name Name of the uniform variable in the shader source.
    * @param mat 4x4 matrix to assign to the uniform.
    */
    void SetUniformMat4(GLuint ShaderID, const std::string& name, const glm::mat4& mat);


    /**
    * @brief Returns the OpenGL handle of the default sprite shader program.
    *
    * Provides access to the internal sprite shader used by DrawImage() and
    * other basic 2D rendering operations.
    *
    * @return GLuint Shader program ID of the sprite shader.
    */
    GLuint GetSpriteShader() const { return shaderProgram; } //for now this, can added instancedShader in the future

    void SetTint(const Colour3& tint);
    void ClearTint();

    void DrawRoundedRect(const Vec2& pos, float width, float height, float radius, const Colour3& color);

private:

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
    // std::unordered_map<GLuint, std::string> texturePathMap;

    
    glm::vec3 currentTint = { 1.f, 1.f, 1.f };
};
