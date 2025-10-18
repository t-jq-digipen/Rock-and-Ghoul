/* ========================================================
 * @file        Render.cpp
 * @author      Kervin Ong Guan Cheng | kervinguancheng.ong@digipen.edu
 * @date        02 OCT 2025
 *
 * @brief   Rendering system class, consisting function definitions for drawing 2D
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

#include "Render.h"
#include "glapp.h"
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// stb_image
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Vertex + Fragment shaders loaded from file in Assets, any changes to shader code to be done inside those files

Render::Render() : vao(0), vbo(0), shaderProgram(0), InstanceShaderProgram(0), InstanceVBO(0), projection(0), textVAO(0), textVBO(0) {}

Render::~Render() {
    // Clean up OpenGL resources
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (shaderProgram) glDeleteProgram(shaderProgram);

    // Clean up text rendering resources
    if (textVAO) glDeleteVertexArrays(1, &textVAO);
    if (textVBO) glDeleteBuffers(1, &textVBO);

    //clean up FreeType character textures
    for (auto& [c, character] : Characters) {
        glDeleteTextures(1, &character.TextureID);
    }
    Characters.clear();

    // Clean up all cached textures
    ClearAllTextures();
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attrib
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Texcoord attrib
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    //====== Instancing render ==========

    //instance VBO (initially empty, will be filled per draw call)
    glGenBuffers(1, &InstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, InstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(InstanceData) * 10000, nullptr, GL_DYNAMIC_DRAW); // Pre-allocate for 10k instances

    // Instance attribute 0, position
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, position));
    glVertexAttribDivisor(2, 1); // This attribute updates per instance

    // Instance attribute 1,  scale 
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, scale));
    glVertexAttribDivisor(3, 1);

    // Instance attribute 2, uvRect
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, uvRect));
    glVertexAttribDivisor(4, 1);

    // Instance attribute 3, rotation
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, rotation));
    glVertexAttribDivisor(5, 1);


    glBindVertexArray(0);

    // Shaders
    shaderProgram = CreateProgramFromFiles(
        "../Assets/vertexSrc.vert",
        "../Assets/fragmentSrc.frag"
    );

    InstanceShaderProgram = CreateProgramFromFiles(
        "../Assets/InstanceVertexSrc.vert",  // try use a new vertex shader for instancing , cus it needs per-instance attributes
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

    // no use texture
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), GL_FALSE);

    //draw
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    glUseProgram(0);
}

void Render::DrawTexturedQuad(const Vec2& pos, const Vec2& scale, GLuint texture, float rotation) {
    glUseProgram(shaderProgram);

    // upload ortho projection
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projection[0][0]);

    // upload pos + scale + rotation
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), pos.x, pos.y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), scale.x, scale.y);
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), rotation); //added rotation

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

    //draw
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

void Render::DrawCircle(const Vec2& center, float radius, const Colour3& color, int segments) {
    // vertices for a circle
    std::vector<GLfloat> vertices;
    vertices.reserve((segments + 2) * 2);

    //center point
    vertices.push_back(center.x);
    vertices.push_back(center.y);

    for (int i = 0; i <= segments; i++) {
        float angle = static_cast<float>((2.0f * 3.1415 * i) / segments);
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

    // Neutral transform values (circle already has actual coords)
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

    //set uniforms
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), pos.x, pos.y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), width, height);
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), color.r, color.g, color.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), rotation);
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), GL_FALSE);

    //draw
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
        throw std::runtime_error("Unable to load texture\n");
        return 0;
    }

    GLenum format = GL_RGB;
    if (channels == 1) { format = GL_RED; }
    else if (channels == 3) { format = GL_RGB; }
    else if (channels == 4) { format = GL_RGBA; }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
        format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // set parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(data);

    // Cache it
    textureCache[path] = textureID;
    pathCache[textureID] = path;

    // json
    //texturePathMap[textureID] = path;

    return textureID;
}

// helpers for json
/*void Render::RegisterTexturePath(GLuint texID, const std::string& path) {
    texturePathMap[texID] = path;
}

std::string Render::GetTexturePath(GLuint texID) const {
    auto it = texturePathMap.find(texID);
    if (it != texturePathMap.end()) {
        return it->second;
    }
    return "";
}*/


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
            return;
        }
        // std::cout << "Warning: Texture ID " << textureID << " not found in cache" << std::endl;
    }
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

    //error check in case
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
    throw std::runtime_error("Unable to retrieve texture path\n");
}

bool Render::IsAreaTouched(const glm::vec2& mousePos, const glm::vec2& areaPos, float width, float height)
{
    //set uniforms
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), areaPos.x, areaPos.y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), width, height);

    const glm::vec2 deltaPos = glm::abs(mousePos - areaPos);
    const float hx = 0.5f * std::abs(width);
    const float hy = 0.5f * std::abs(height);

    return (deltaPos.x <= hx) && (deltaPos.y <= hy);
}

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

void Render::Draw_SpecImage(const Image& image, float x, float y, float w, float h, float rotation) {
    if (!image.IsValid()) {
        return;
    }

    Vec2 pos(x, y);
    Vec2 scale(w, h);
    DrawTexturedQuad(pos, scale, image.textureID, rotation);
}

void Render::DrawImageScaled(const Image& image, float x, float y, float scaleX, float scaleY, float rotation) {
    if (!image.IsValid()) {
        return;
    }

    float w = image.widthPixels * scaleX;
    float h = image.heightPixels * scaleY;
    Draw_SpecImage(image, x, y, w, h, rotation);
}

void Render::DrawImageRotated(const Image& image, float x, float y, float angleRad) {
    if (!image.IsValid()) {
        return;
    }

    glUseProgram(shaderProgram);

    // upload projection / state that the vert/frag shader expects
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

    //draw
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);   
    glBindVertexArray(0);

    glUseProgram(0);
}

void Render::DrawImageSub(const Image& Image,
    float x, float y, float w, float h,
    float u0, float v0, float u1, float v1, float rotation)
{
    glUseProgram(shaderProgram);

    //Projection
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, glm::value_ptr(projection));

    //Position, scale, rotation uniforma
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), x, y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), w, h);
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), rotation); // if you want no rotation

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
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), rotation);

    //Texture binding
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Image.textureID);
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

    // Tell shader we be using texture mode
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), true);
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseText"), false);

    // Draw quad 
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    glUniform1i(glGetUniformLocation(shaderProgram, "uUseSubImage"), false); //tell shader ok i am done using sub image mode*
}


int Render::GetImageWidth(const Image& image) const {
    return image.widthPixels;
}

int Render::GetImageHeight(const Image& image) const {
    return image.heightPixels;
}


//instancing
void Render::DrawImageInstanced(const Image& image, const std::vector<InstanceData>& instances) {
    if (instances.empty() || !image.IsValid()) {
        return;
    }

    // Upload instance data
    glBindBuffer(GL_ARRAY_BUFFER, InstanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instances.size() * sizeof(InstanceData), instances.data());

    // Use shader (INSTANCE)
    glUseProgram(InstanceShaderProgram);

    // Set uniforms
    GLint projLoc = glGetUniformLocation(InstanceShaderProgram, "uProjection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, image.textureID);

    GLint texLoc = glGetUniformLocation(InstanceShaderProgram, "uTexture");
    glUniform1i(glGetUniformLocation(InstanceShaderProgram, "uUseTexture"), true); //enable this to see texture
    glUniform1i(texLoc, 0);

    // Draw
    glBindVertexArray(vao);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, static_cast<GLsizei>(instances.size()));

    glBindVertexArray(0);
}


void Render::DrawRectInstanced(const std::vector<InstanceData>& instances, const Colour3& color) {
    if (instances.empty()) {
        return;
    }

    // Upload instance data
    glBindBuffer(GL_ARRAY_BUFFER, InstanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instances.size() * sizeof(InstanceData), instances.data());

    glUseProgram(InstanceShaderProgram);

    // Projection
    GLint projLoc = glGetUniformLocation(InstanceShaderProgram, "uProjection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

    // Pass solid color instead of texture (turn off use texture)
    GLint useTexLoc = glGetUniformLocation(InstanceShaderProgram, "uUseTexture");
    glUniform1i(useTexLoc, false);

    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), color.r, color.g, color.b);

    // Draw instanced quad
    glBindVertexArray(vao);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, static_cast<GLsizei>(instances.size())); //instanced
    glBindVertexArray(0);
}

//text rendering
void Render::InitTextRendering() {
    // If InitTextRendering can be called multiple times,
    // first clear any previous GPU objects it created.
    if (!Characters.empty()) {
        for (auto& [c, ch] : Characters) {
            glDeleteTextures(1, &ch.TextureID);
        }
        Characters.clear();
    }
    if (textVAO) {
        glDeleteVertexArrays(1, &textVAO);
        textVAO = 0;
    }
    if (textVBO) {
        glDeleteBuffers(1, &textVBO);
        textVBO = 0;
    }

    //FreeType lifetime with single exit
    FT_Library ft = nullptr;
    FT_Face face = nullptr;

    auto cleanupFT = [&]() {
        if (face) { FT_Done_Face(face); face = nullptr; }
        if (ft) { FT_Done_FreeType(ft); ft = nullptr; }
        };

    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        cleanupFT();
        return;
    }

    if (FT_New_Face(ft, "../Assets/Antonio-Bold.ttf", 0, &face)) { //for now, direct path is placed in here
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        cleanupFT();
        return;
    }

    // Font size + unpack alignment for glyph textures
    FT_Set_Pixel_Sizes(face, 0, 48);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // glyph bitmaps are byte-aligned

    // Load first 128 ASCII glyphs
    for (unsigned char c = 0; c < 128; ++c) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYPE: Failed to load Glyph '" << (int)c << "'\n";
            continue;  
        }

        unsigned int texture = 0;
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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //GL_NEAREST
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width,  face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left,   face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };

        // If character already existed (re-init), delete old texture first (we cleared above though)
        Characters[c] = character;
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    // Always release FreeType objects on every path
    cleanupFT();

    // VAO/VBO for text quads 
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    // aPos (x,y)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // aTex (u,v)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void Render::DrawText(std::string text, float x, float y, float scale, const Colour3& color) {

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // activate render state	
    glUseProgram(shaderProgram);

    //set uniforms
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"), 1, GL_FALSE, &projection[0][0]); 
    glUniform3f(glGetUniformLocation(shaderProgram, "uTextColor"), color.r, color.g, color.b);
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseText"), GL_TRUE);
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

    //text no need, so set them transform uniforms to defaults
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
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // use glBufferSubData 
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // advance cursors for next glyph (advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUniform1i(glGetUniformLocation(shaderProgram, "uUseText"), GL_FALSE); //Remember tell shader ok im done using text
}


void Render::DrawTextBox(const std::string& text, float x, float y, float rowWidth, float scale, const Colour3& color) {

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // activate render state	
    glUseProgram(shaderProgram);

    //set uniforms
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3f(glGetUniformLocation(shaderProgram, "uTextColor"), color.r, color.g, color.b);
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseText"), GL_TRUE);
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

    //text no need, so set them transform uniforms to defaults
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), 0.0f, 0.0f);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), 0.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    //track where next glyph will placed
    float nextX = x;
    float nextY = y;

    //get the horizontal step for space char
    float spaceWidth = (Characters[' '].Advance >> 6) * scale;

    std::istringstream iss(text);
    std::string word;

    while (iss >> word) {

        // Measure word width
        float wordWidth = 0.0f;

        for (char c : word) {
            Character ch = Characters[c];
            wordWidth += (ch.Advance >> 6) * scale;
        }

        // Wrap to new line
        //Check if drawing the whole word will excced the right edge
        if (nextX + wordWidth > x + rowWidth) {
            //if yes reset the nextX to be the left edge
            nextX = x;
            nextY -= (Characters['H'].Size.y * 1.5f) * scale; //move down like new line
        }

        // Draw characters of word (similar setup to previous)
        for (char c : word) {
            Character ch = Characters[c];

            float xpos = nextX + ch.Bearing.x * scale;
            float ypos = nextY - (ch.Size.y - ch.Bearing.y) * scale;
            float w = ch.Size.x * scale;
            float h = ch.Size.y * scale;

            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },

                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }
            };

            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            glBindBuffer(GL_ARRAY_BUFFER, textVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            nextX += (ch.Advance >> 6) * scale; //position next char 
        }

        nextX += spaceWidth; // Add space
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUniform1i(glGetUniformLocation(shaderProgram, "uUseText"), GL_FALSE); //Remember tell shader ok im done using text

}


//Load all image assets from JSON file (containing the image paths)
void Render::LoadImageAssetsFromJSON(const std::string& jsonPath) {

    //maybe can default create a json for image if doesnt exists?
    
    //read file
    std::ifstream file{ jsonPath };
    //check if the json file for images exists
    if (!file.is_open()) {
        std::cerr << "Failed to open Image Asset JSON: " << jsonPath << std::endl;
        return;
    }

    json j;
    try {
        file >> j;
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing Image JSON file: " << e.what() << std::endl;
        return;
    }

    //"images" is the name of the array in the json (if u edit the name in the file, need remember update here)
    if (!j.contains("images") || !j["images"].is_array()) {
        std::cerr << "Invalid JSON format: missing \"images\" array in " << jsonPath << std::endl;
        return;
    }

    //for my sanity to check if the loaded amount tally
    int loadedCount = 0;

    //for each entry in the images array...
    for (const auto& path : j["images"]) {

        //ignores if entry is not a string
        if (!path.is_string()) {
            std::cerr << "Skipping invalid image entry (not a string)" << std::endl;
            continue;
        }

        std::string texPath = path.get<std::string>();

        // Check if the file actually exists
        if (!fs::exists(texPath)) {
            std::cerr << "Warning: Texture file not found: " << texPath << std::endl;
            continue; // Skip this one
        }

        //Loading and caching stuff and checks
        /*if (textureCache.find(texPath) != textureCache.end()) {
            std::cout << "Already cached: " << texPath << std::endl;
            continue;
        }*/

        //Load the texture
        GLuint texID = LoadTexture(texPath);

        loadedCount++;

    }

    //just checking if logic works
    std::cout << "[Render] Loaded " << loadedCount << " textures from " << jsonPath << std::endl;
}

void Render::Shutdown() {
    // Delete all glyph textures
    for (auto& [c, ch] : Characters) {
        glDeleteTextures(1, &ch.TextureID);
    }
    Characters.clear();

    // Delete text rendering VAO/VBO
    if (textVAO) {
        glDeleteVertexArrays(1, &textVAO);
        textVAO = 0;
    }
    if (textVBO) {
        glDeleteBuffers(1, &textVBO);
        textVBO = 0;
    }

    // Delete main VAO/VBO
    if (vao) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    if (vbo) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }

    // Delete shader program
    if (shaderProgram) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }

    // Clear cached textures
    ClearAllTextures();

    std::cout << "[Render] Shutdown complete — all GPU resources freed.\n";
}
