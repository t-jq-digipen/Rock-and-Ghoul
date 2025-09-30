#include "Render.h"
#include "glapp.h"
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
Render* Render::instance = nullptr;


// stb_image
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Vertex + Fragment shaders loaded from file in Assets, any changes to shader code to be done inside those fiels

Render::Render() : vao(0), vbo(0), shaderProgram(0) {}

Render::~Render() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (shaderProgram) glDeleteProgram(shaderProgram);
    ClearAllTextures();
    //ClearAllImages();
}

void Render::Init() {
    // Square vertices (pos + uv)
    float vertices[] = {
        //   x,    y,   u,  v
        -0.5f, -0.5f, 0.0f, 0.0f,
         0.5f, -0.5f, 1.0f, 0.0f,
         0.5f,  0.5f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 1.0f
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //was GL_STATIC_DRAW previously

    // Position attrib
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Texcoord attrib
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    // Shaders
    shaderProgram = CreateProgramFromFiles(
        "../Assets/vertexSrc.vert",
        "../Assets/fragmentSrc.frag"
    );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Setup projection
    projection = glm::ortho(
        0.0f, static_cast<float>(GLApp::width),
        0.0f, static_cast<float>(GLApp::height),
        -1.0f, 1.0f
    );

    InitTextRendering();
}

void Render::ClearBackground(const Colour4& color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

//draw stuff
void Render::DrawSquare(const glm::vec2& pos, float size, const glm::vec3& color, float rotation) {
    glUseProgram(shaderProgram);

    // upload projection
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projection[0][0]);

    // upload pos, size (use size for both X and Y), color
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), pos.x, pos.y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), size, size);
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), rotation);
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), color.r, color.g, color.b);

    // no texture
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), GL_FALSE);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    glUseProgram(0);
}

void Render::DrawTexturedQuad(const Vec2& pos, const Vec2& scale, GLuint texture, float rotation) {
    glUseProgram(shaderProgram);

    // upload projection
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projection[0][0]);

    // upload pos + scale
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), pos.x, pos.y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), scale.x, scale.y);
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), rotation); //added rotation
    //glUniform4f(glGetUniformLocation(shaderProgram, "uUVRect"), 0.0f, 0.0f, 1.0f, 1.0f); 


    // enable texture
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), GL_TRUE);

    // bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

    // draw
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    glUseProgram(0);
}

void Render::DrawLine(const Vec2& start, const Vec2& end, const Colour3& color) {
    GLfloat vertices[4] = {
        start.x, start.y,
        end.x,   end.y
    };

    GLuint vboLine, vaoLine;
    glGenVertexArrays(1, &vaoLine);
    glGenBuffers(1, &vboLine);

    glBindVertexArray(vaoLine);
    glBindBuffer(GL_ARRAY_BUFFER, vboLine);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glUseProgram(shaderProgram);

    // Set required uniforms
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projection[0][0]);
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), color.r, color.g, color.b);
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), GL_FALSE);
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), 0.0f);
    // Set pos and scale to neutral values so vertices pass through unchanged
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), 0.0f, 0.0f);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), 1.0f, 1.0f);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);

    glDrawArrays(GL_LINES, 0, 2);
    glLineWidth(3.0f);
    // cleanup
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &vboLine);
    glDeleteVertexArrays(1, &vaoLine);

    glUseProgram(0);
}

void Render::DrawCircle(const Vec2& center, float radius, const Colour3& color, int segments, float rotation) {
    // vertices for a circle
    std::vector<GLfloat> vertices;
    vertices.reserve((segments + 2) * 2);

    //ctr point
    vertices.push_back(center.x);
    vertices.push_back(center.y);
    float rot = rotation;

    for (int i = 0; i <= segments; i++) {
        float angle = (2.0f * 3.1415 * i) / static_cast<float>(segments);
        float ang = angle + rot;
        float x = center.x + radius * cos(angle);
        float y = center.y + radius * sin(angle);
        vertices.push_back(x);
        vertices.push_back(y);
    }

    GLuint vaoCircle, vboCircle;
    glGenVertexArrays(1, &vaoCircle);
    glGenBuffers(1, &vboCircle);

    glBindVertexArray(vaoCircle);
    glBindBuffer(GL_ARRAY_BUFFER, vboCircle);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glUseProgram(shaderProgram);

    // Upload uniforms
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projection[0][0]);
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), color.r, color.g, color.b);
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), GL_FALSE);
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), 0.0f);

    // Unbind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Neutral transform (circle already has actual coords)
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), 0.0f, 0.0f);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), 1.0f, 1.0f);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);

    // circle using TRIANGLE_FAN
    glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);

    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &vboCircle);
    glDeleteVertexArrays(1, &vaoCircle);

    glUseProgram(0);
}  // <-- Make sure this closing brace is here!

void Render::DrawRect(const Vec2& pos, float width, float height, const Colour3& color, float rotation) {
    glUseProgram(shaderProgram);

    // upload projection
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projection[0][0]);

    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), pos.x, pos.y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), width, height);
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), color.r, color.g, color.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), rotation);
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), GL_FALSE);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    glUseProgram(0);
}



GLuint Render::LoadTexture(const std::string& path) {

    // Check if texture already exists
    auto it = textureCache.find(path);
    if (it != textureCache.end()) {
        std::cout << "Found in cache: " << path << " ID: " << it->second << std::endl;
        return it->second; // Return cached texture
    }

    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    std::cout << "Loading texture: " << path
        << " (" << width << "x" << height
        << ", channels=" << channels << ")" << std::endl;
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        throw std::runtime_error("TEXTURE CANNOT LOAD LA BOBO\n");
        return 0;
    }

    GLenum format = GL_RGB;
    if (channels == 1) format = GL_RED;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 4) format = GL_RGBA;

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
        format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // set parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    // Cache it
    textureCache[path] = textureID;
    pathCache[textureID] = path;

    // json
    texturePathMap[textureID] = path;

    return textureID;
}

// helpers for json
void Render::RegisterTexturePath(GLuint texID, const std::string& path) {
    texturePathMap[texID] = path;
}

std::string Render::GetTexturePath(GLuint texID) const {
    auto it = texturePathMap.find(texID);
    if (it != texturePathMap.end()) {
        return it->second;
    }
    return "";
}
// ====================
void Render::UnloadTexture(const std::string& path) {
    auto it = textureCache.find(path);
    if (it != textureCache.end()) {
        pathCache.erase(it->second);
        glDeleteTextures(1, &it->second);
        textureCache.erase(it);
    }
}

void Render::UnloadTexture(GLuint textureID) {
    // Find the texture in the cache
    for (auto it = textureCache.begin(); it != textureCache.end(); ++it) {
        if (it->second == textureID) {
            std::cout << "Unloading texture ID: " << textureID << " from path: " << it->first << std::endl;
            pathCache.erase(it->second);
            glDeleteTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, 0); // Unbind any texture
            textureCache.erase(it);
            std::cout << "Texture unloaded successfully" << std::endl;
            return; // Found and removed, exit
        }
    }
    std::cout << "Warning: Texture ID " << textureID << " not found in cache" << std::endl;
}

void Render::ClearAllTextures() {
    for (auto& pair : textureCache) {
        glDeleteTextures(1, &pair.second);
    }
    pathCache.clear();
    textureCache.clear();
}


std::string Render::readTextFile(const std::string& path) {

    std::ifstream ifs(path, std::ios::in | std::ios::binary);

    if (!ifs) {
        /*
        std::cout << "cwd = " << std::filesystem::current_path() << "\n";
        std::cout << "trying to open: " << path << "\n";
        std::cout << "exists? " << std::filesystem::exists(path) << "\n";*/
        throw std::runtime_error("Cannot open file: " + path);

    }

    std::string s;
    ifs.seekg(0, std::ios::end);
    s.resize(static_cast<size_t>(ifs.tellg()));
    ifs.seekg(0, std::ios::beg);
    ifs.read(&s[0], s.size());

    return s;
}


GLuint Render::CompileShaderFromFile(const std::string& path, GLenum type) {

    const std::string src = readTextFile(path);
    const char* csrc = src.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &csrc, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);

    if (!ok) {
        char log[2048];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader compile error in \"" << path << "\":\n" << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint Render::CreateProgramFromFiles(const std::string& vertPath, const std::string& fragPath) {

    GLuint vsh = CompileShaderFromFile(vertPath, GL_VERTEX_SHADER);
    GLuint fsh = CompileShaderFromFile(fragPath, GL_FRAGMENT_SHADER);
    if (!vsh || !fsh) {
        if (vsh) {
            glDeleteShader(vsh);
        }
        if (fsh) {
            glDeleteShader(fsh);
        }
        return 0;
    }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vsh);
    glAttachShader(prog, fsh);
    glLinkProgram(prog);

    glDeleteShader(vsh);
    glDeleteShader(fsh);

    GLint ok = GL_FALSE;

    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        std::cerr << "Program link error:\n" << log << std::endl;
        glDeleteProgram(prog);
        return 0;
    }

    return prog;
}

std::string Render::Get_TexturePath(const GLuint& imgID)
{
    auto it = pathCache.find(imgID);
    if (it != pathCache.end())
    {
        return it->second;
    }
    throw std::runtime_error("NO TEXTURE PATH??? WTF GO ASK JQ\n");
    return{};
}


bool Render::IsAreaTouched(const glm::vec2& mousePos, const glm::vec2& areaPos, float width, float height)
{
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), areaPos.x, areaPos.y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), width, height);
    const glm::vec2 deltaPos = glm::abs(mousePos - areaPos);
    const float hx = 0.5f * std::abs(width);
    const float hy = 0.5f * std::abs(height);
    return (deltaPos.x <= hx) && (deltaPos.y <= hy);
}



////Image stuff
//Image Render::LoadImage(const std::string& path) {
//    // Check if already in image cache
//    auto it = imageCache.find(path);
//    if (it != imageCache.end()) {
//        std::cout << "Image found in cache: " << path << std::endl;
//        return it->second;
//    }
//
//    // Load texture using existing method
//    GLuint textureID = LoadTexture(path);
//    if (textureID == 0) {
//        std::cout << "Failed to load image: " << path << std::endl;
//        return Image(); // Return invalid image
//    }
//
//    // Get texture dimensions
//    int width, height;
//    glBindTexture(GL_TEXTURE_2D, textureID);
//    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
//    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
//    glBindTexture(GL_TEXTURE_2D, 0);
//
//    Image image = CreateImageFromTexture(textureID, width, height, path);
//    imageCache[path] = image; //store in cache
//
//    std::cout << "Loaded image: " << path << " (" << width << "x" << height << ")" << std::endl;
//    return image;
//}

//Image stuff
Image CreateImage(const std::string& path) {

    // Load texture using existing method 
    // Will give you existing GLuint if exists in cache,
    // otherwise, creates a new GLuint, caches it and the path,
    // then returns you the GLuint of the new texture
    GLuint textureID = Render::GetInstance().LoadTexture(path);
    if (textureID == 0) {
        std::cout << "Failed to load image: " << path << std::endl;
        return Image(); // Return invalid image
    }

    // Get texture dimensions
    int width, height;
    glBindTexture(GL_TEXTURE_2D, textureID);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    glBindTexture(GL_TEXTURE_2D, 0);

    Image temp = { textureID, width, height };

    std::cout << "Loaded image: " << path << " (" << width << "x" << height << ")" << std::endl;
    return temp;
}

void Render::DrawImage(const Image& image, float x, float y) {
    if (!image.IsValid()) {
        return;
    }

    Vec2 pos(x, y);
    Vec2 scale(static_cast<float>(image.widthPixels), static_cast<float>(image.heightPixels));
    DrawTexturedQuad(pos, scale, image.textureID);
}

void Render::Draw_SpecImage(const Image& image, float x, float y, float w, float h) {
    if (!image.IsValid()) {
        return;
    }

    Vec2 pos(x, y);
    Vec2 scale(w, h);
    DrawTexturedQuad(pos, scale, image.textureID);
}

void Render::DrawImageScaled(const Image& image, float x, float y, float scaleX, float scaleY) {
    if (!image.IsValid()) {
        return;

    }

    float w = image.widthPixels * scaleX;
    float h = image.heightPixels * scaleY;
    Draw_SpecImage(image, x, y, w, h);
}

void Render::DrawImageRotated(const Image& image, float x, float y, float angleRad) {
    if (!image.IsValid()) return;

    glUseProgram(shaderProgram);

    // upload projection / state that the frag shader expects
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projection[0][0]);
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), GL_TRUE);
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

    // upload transform
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), x, y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), (float)image.widthPixels, (float)image.heightPixels);
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), angleRad);

    // bind texture + draw
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, image.textureID);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);   //arrays, not elements
    glBindVertexArray(0);

    glUseProgram(0);
}

void Render::DrawImageSub(const Image& Image,
    float x, float y, float w, float h,
    float u0, float v0, float u1, float v1)
{
    glUseProgram(shaderProgram);

    //Projection
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, glm::value_ptr(projection));

    //Position, scale, rotation uniforma
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), x, y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), w, h);
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), 0.0f); // if you want no rotation

    //Sub-image UV rectangle (normalize with texture size)
    int texW, texH;
    glBindTexture(GL_TEXTURE_2D, Image.textureID);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texW);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texH);

    float u0n = u0 / float(texW);
    float v0n = v0 / float(texH);
    float u1n = u1 / float(texW);
    float v1n = v1 / float(texH);

    glUniform1i(glGetUniformLocation(shaderProgram, "uUseSubImage"), true); //tell shader i want to use sub image mode
    glUniform4f(glGetUniformLocation(shaderProgram, "uUVRect"), u0n, v0n, u1n, v1n); //sub mode uses this uniform

    //Texture binding
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Image.textureID);
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

    // Tell shader we’re using texture mode
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), true);
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseText"), false);

    // Draw quad (VAO should be a unit quad with aPos and aTex in [0,1])
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    //Important, to make sure the cache doesnt use the cropped version, cus if u use the sub image previously, the cache will store the sub image everyth will become cropped
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseSubImage"), false); //tell shader ok i am done using sub image mode
}



int Render::GetImageWidth(const Image& image) const {
    return image.widthPixels;
}

int Render::GetImageHeight(const Image& image) const {
    return image.heightPixels;
}

//void Render::UnloadImage(Image& image) {
//    if (!image.IsValid()) {
//        return;
//    }
//
//    // Remove from cache
//    imageCache.erase(image.path);
//
//    // Unload the texture
//    UnloadTexture(image.textureID);
//
//    // Invalidate the image
//    image.textureID = 0;
//    image.width = 0;
//    image.height = 0;
//    image.path.clear();
//
//    std::cout << "Unloaded image: " << image.path << std::endl;
//}

//void Render::ClearAllImages() {
//    for (auto& pair : imageCache) {
//        UnloadTexture(pair.second.textureID);
//    }
//    imageCache.clear();
//    std::cout << "All images cleared from cache" << std::endl;
//}

//Image Render::CreateImageFromTexture(GLuint textureID, int width, int height, const std::string& path) {
//    return Image(textureID, width, height, path);
//}


//text rendering
void Render::InitTextRendering() {

    // FreeType
    // --------
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    // find path to font (leave this here first after filepath is implemented
    /*std::string font_name = FilePath::getPath("resources/fonts/Antonio-Bold.ttf");
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return;
    }*/

    FT_Face face;
    if (FT_New_Face(ft, "../Assets/Antonio-Bold.ttf", 0, &face)) //for now use this direct full file path name, note to download and add the font type inside Assets,
        //thinking maybe can haev set_font function to pull path (basically the above commented stuff
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return;
        //exit(0);
    }
    else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set, if want arrow or smth need increase the chars i think
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }

            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );

            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use

            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };

            Characters.insert(std::pair<char, Character>(c, character));
        }

        glBindTexture(GL_TEXTURE_2D, 0);

    }

    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    //glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);

    // configure VAO/VBO for texture quads
    // -----------------------------------
    //GLuint textVAO, textVBO;
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    //glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0); <--- doesnt matches the vert shader aTex, aPos, x,y,u,v
    // Fix vertex attributes
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    //===========================================================================


}

void Render::DrawText(std::string text, float x, float y, float scale, const Colour3& color) {

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // activate render state	
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"), 1, GL_FALSE, &projection[0][0]); //set projection val, using the one previously defined in init
    glUniform3f(glGetUniformLocation(shaderProgram, "uTextColor"), color.r, color.g, color.b);
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseText"), GL_TRUE);
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

    // For text, we don't use transformation uniforms, so set them to defaults
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), 0.0f, 0.0f);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), 0.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);


    // iterate through all characters
    std::string::const_iterator c;

    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        // Make sure vertex attributes are set correctly

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUniform1i(glGetUniformLocation(shaderProgram, "uUseText"), GL_FALSE); //REMEMBER TO SET THIS BACK TO FALSE, CUS REM U SET IT TO TRUE TO TRIGGER uUseText in the beginning
}