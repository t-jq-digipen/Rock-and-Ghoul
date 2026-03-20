/* ========================================================
 * @file        Render.cpp
 * @author      Kervin Ong Guan Cheng | kervinguancheng.ong@digipen.edu
 * @date        05 NOV 2025
 *
 * @brief   Rendering system class, consisting function definitions for drawing 2D
 *          primitives, images, and text using OpenGL.
 *
 * The Render class provides a central interface for all rendering
 * operations, including shapes, images, instanced rendering, text
 * rendering, and scene capture. It manages OpenGL resources such as
 * shaders, framebuffers, textures, and VAOs, while offering a simple
 * API for use by game systems and tools.
 *
 * Functionalities include:
 * - Drawing shapes (rectangles, circles, lines)
 * - Loading, caching, and unloading textures
 * - Drawing images (full, scaled, sub-regions, rotated)
 * - Batched/Instanced rendering for performance optimization
 * - Text rendering using FreeType
 * - Off-screen rendering via scene framebuffer (for ImGui or post-processing)
 * - Screen-to-world coordinate conversion utilities
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "FilePath.h"
#include "Render.h"
#include "glapp.h"
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


 // stb_image
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <Camera.h>

// Vertex + Fragment shaders loaded from file in Assets, any changes to shader code to be done inside those files
//extern Camera mainCamera;
Camera& mainCamera = Camera::GetInstance();

//Render::Render() : vao(0), vbo(0), shaderProgram(0), InstanceShaderProgram(0), InstanceVBO(0), projection(0), textVAO(0), textVBO(0) {}
Render::Render() : 
    vao(0), vbo(0), 
    shaderProgram(0), InstanceShaderProgram(0), InstanceVBO(0), projection(0), 
    textVAO(0), textVBO(0), 
    batchVAO(0), batchVBO(0), batchEBO(0), batchShader(0),
    vertVAO(0), vertVBO(0) {}

Render::~Render() {
    // Clean up OpenGL resourcess
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


    //============ Allocate and setup buffers for custom verts ===============
    glGenVertexArrays(1, &vertVAO);
    glGenBuffers(1, &vertVBO);

    glBindVertexArray(vertVAO);
    glBindBuffer(GL_ARRAY_BUFFER, vertVBO);

    // Allocate buffer for 4 vertices (cause use dynamic draw for this)
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * 4, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (void*)0);

    glBindVertexArray(0);

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

    // Shaders — deferred to LoadShaders() so Init() is safe with no game loaded
    LoadShaders();

    /*InstanceShaderProgram = CreateProgramFromFiles(
        (FilePath::ASSETS_FOLDER + "InstanceVertexSrc.vert"),  // try use a new vertex shader for instancing , cus it needs per-instance attributes
        (FilePath::ASSETS_FOLDER + "fragmentSrc.frag")
    );*/


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Setup projection
    UpdateProjection();
    InitTextRendering();

    InitBatch(); //uncomment once batch shaders are ready

    // --- create framebuffer for editor viewport ---
    glGenFramebuffers(1, &sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

    // color texture
    glGenTextures(1, &sceneColor);
    glBindTexture(GL_TEXTURE_2D, sceneColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, GLApp::width, GLApp::height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneColor, 0);

    // depth-stencil buffer
    glGenRenderbuffers(1, &sceneDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, sceneDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, GLApp::width, GLApp::height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sceneDepth);

    // verify
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "[Render] Scene framebuffer incomplete!\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Render::ClearBackground(const Colour4& color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}
void Render::UpdateProjection(bool useVirtual)
{
    if (useVirtual) {
        projection = glm::ortho(
            0.0f, static_cast<float>(GLApp::VIRTUAL_W),
            0.0f, static_cast<float>(GLApp::VIRTUAL_H),
            -1.0f, 1.0f
        );
    } else {
        projection = glm::ortho(
            0.0f, static_cast<float>(GLApp::width),
            0.0f, static_cast<float>(GLApp::height),
            -1.0f, 1.0f
        );
    }
}

void Render::SetProjectionType(ProjectionType type)
{
    GLuint shader = GetSpriteShader();
    glUseProgram(shader);

    glm::mat4 proj;

    switch (type)
    {
    case ProjectionType::WORLD:
        proj = mainCamera.GetViewProjection();
        break;

    case ProjectionType::UI:
        proj = glm::ortho(
            0.f, (float)GLApp::VIRTUAL_W,
            0.f, (float)GLApp::VIRTUAL_H,
            -1.f, 1.f
        );
        break;
    }

    glUniformMatrix4fv(
        glGetUniformLocation(shader, "uProjection"),
        1, GL_FALSE,
        &proj[0][0]
    );

    glUseProgram(0);
}



void Render::DrawSquare(const glm::vec2& pos, float size, const glm::vec3& color, float rotation) {
    glUseProgram(shaderProgram);

    // upload projection
    /*glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projection[0][0]);*/

    glm::mat4 projMatrix = useCamera
        ? mainCamera.GetViewProjection()
        : projection;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projMatrix[0][0]);

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
    /*glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projection[0][0]);*/
        /*glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
            1, GL_FALSE, &mainCamera.GetViewProjection()[0][0]); //testing move bg */

            //added this one for camera, need pass camera view projection matrix 
    glm::mat4 projMatrix = useCamera
        ? mainCamera.GetViewProjection()
        : projection;

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projMatrix[0][0]);

    // upload pos + scale + rotation
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), pos.x, pos.y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), scale.x, scale.y);
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), rotation); //added rotation
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), 1.0, 1.0, 1.0);

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
    glUseProgram(shaderProgram);

    // Set line width FIRST
    glLineWidth(3.0f);

    // Upload projection
    glm::mat4 projMatrix = useCamera
        ? mainCamera.GetViewProjection()
        : projection;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projMatrix[0][0]);

    // Set uniforms - match what DrawRect does
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), color.r, color.g, color.b);
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), GL_FALSE);
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), 0.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "uAlpha"), 1.0f);
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), 0.0f, 0.0f);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), 1.0f, 1.0f);

    // Create vertices with BOTH position and texcoord (even though we don't use texcoord)
    // This matches the layout expected by the main vao
    float vertices[] = {
        start.x, start.y, 0.0f, 0.0f,  // pos + uv
        end.x,   end.y,   1.0f, 1.0f   // pos + uv
    };

    GLuint tempVAO, tempVBO;
    glGenVertexArrays(1, &tempVAO);
    glGenBuffers(1, &tempVBO);

    glBindVertexArray(tempVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Texcoord attribute (location 1) - even though unused
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // Draw
    glDrawArrays(GL_LINES, 0, 2);

    // Cleanup
    glBindVertexArray(0);
    glDeleteBuffers(1, &tempVBO);
    glDeleteVertexArrays(1, &tempVAO);

    // Reset state
    glLineWidth(1.0f);
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), 1.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "uAlpha"), 1.0f);

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
    /*glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projection[0][0]);*/
        /*glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
            1, GL_FALSE, &mainCamera.GetViewProjection()[0][0]); //testing roundy atm*/

    glm::mat4 projMatrix = useCamera
        ? mainCamera.GetViewProjection()
        : projection;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projMatrix[0][0]);

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

    // Reset color so next sprite draw isn't tinted
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), 1.0f, 1.0f, 1.0f);


    glUseProgram(0);
}  // <-- Make sure this closing brace is here!

void Render::DrawRect(const Vec2& pos, float width, float height, const Colour3& color, float rotation, float alpha) {
    glUseProgram(shaderProgram);

    // upload projection
    /*glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projection[0][0]);*/
        /*glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
            1, GL_FALSE, &mainCamera.GetViewProjection()[0][0]);*/
    glm::mat4 projMatrix = useCamera
        ? mainCamera.GetViewProjection()
        : projection;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projMatrix[0][0]);

    //set uniforms
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), pos.x, pos.y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), width, height);
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), color.r, color.g, color.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), rotation);
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), GL_FALSE);
    glUniform1f(glGetUniformLocation(shaderProgram, "uAlpha"), alpha);


    //draw
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    // Reset color so next sprite draw isn't tinted
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), 1.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "uAlpha"), 1.0f);

    glUseProgram(0);
}


void Render::DrawCustomQuad(const Vec2& v0, const Vec2& v1, const Vec2& v2, const Vec2& v3, const Colour3& color)
{
    glUseProgram(shaderProgram);

    // Upload matrix like usual
    glm::mat4 projMatrix = useCamera
        ? mainCamera.GetViewProjection()
        : projection;

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projMatrix[0][0]);

    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"),
        color.r, color.g, color.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "uAlpha"), 1.0f);

    glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), GL_FALSE);
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseCustomVerts"), GL_TRUE);

    // Upload custom quad vertices
    Vec2 vertices[4] = { v0, v1, v2, v3 };

    /*GLuint tempVBO, tempVAO;
    glGenVertexArrays(1, &tempVAO);
    glGenBuffers(1, &tempVBO);

    glBindVertexArray(tempVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (void*)0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    //glFlush();

    glBindVertexArray(0);
    glDeleteBuffers(1, &tempVBO);
    glDeleteVertexArrays(1, &tempVAO);*/

    //upload v ertices to VBO
    glBindBuffer(GL_ARRAY_BUFFER, vertVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    //bind VAO
    glBindVertexArray(vertVAO);

    //Draw
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    //Cleanup
    glBindVertexArray(0);

    //reset for future draw calls 
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseCustomVerts"), GL_FALSE);
    // Reset color so next sprite draw isn't tinted
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), 1.0f, 1.0f, 1.0f);

    glUseProgram(0);
}



GLuint Render::LoadTexture(const std::string& path) {
    // Resolve legacy "../Assets/..." paths from old JSON files
    const std::string resolvedPath = FilePath::Resolve(path);

    // Check if texture already exists
    auto it = textureCache.find(resolvedPath);
    if (it != textureCache.end()) {
        //std::cout << "Found in cache: " << path << " ID: " << it->second << std::endl;
        return it->second; // Return cached texture
    }

    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(resolvedPath.c_str(), &width, &height, &channels, 0);
   /* std::cout << "Loading texture: " << resolvedPath
        << " (" << width << "x" << height
        << ", channels=" << channels << ")" << std::endl;*/
    if (!data) {
        std::cerr << "Failed to load texture: " << resolvedPath << std::endl;
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
    textureCache[resolvedPath] = textureID;
    pathCache[textureID] = resolvedPath;

    return textureID;
}


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

bool Render::IsAreaTouched(const Vec2& mousePos, const Vec2& areaPos, float width, float height)
{
    //set uniforms
  /*  glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), areaPos.x, areaPos.y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), width, height);

    const glm::vec2 deltaPos = glm::abs(mousePos - areaPos);
    const float hx = 0.5f * std::abs(width);
    const float hy = 0.5f * std::abs(height);

    return (deltaPos.x <= hx) && (deltaPos.y <= hy);*/

    Vec2 MousePos = { mousePos.x, static_cast<float>(GLApp::height) - mousePos.y };

    const Vec2 delta = MousePos - areaPos;
    const float hx = 0.5f * width;
    const float hy = 0.5f * height;

    return (std::abs(delta.x) <= hx) && (std::abs(delta.y) <= hy);
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

   // std::cout << "Loaded image: " << path << " (" << width << "x" << height << ")" << std::endl;
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

void Render::DrawImageRotated(const Image& image, float x, float y, float w, float h, float angleRad, float brightness) {
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
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), w, h);
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), angleRad);
    glUniform1f(glGetUniformLocation(shaderProgram, "uBrightness"), brightness);

    // bind texture + draw
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, image.textureID);

    //draw
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    glUniform1f(glGetUniformLocation(shaderProgram, "uBrightness"), 0.f);

    glUseProgram(0);
}

void Render::DrawImageSub(const Image& Image,
    float x, float y, float w, float h,
    float u0, float v0, float u1, float v1, float rotation, float brightness)
{
    glUseProgram(shaderProgram);

    //Projection
    /*glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, glm::value_ptr(projection));*/
    glm::mat4 projMatrix = useCamera
        ? mainCamera.GetViewProjection()
        : projection;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projMatrix[0][0]);

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
    glUniform1f(glGetUniformLocation(shaderProgram, "uBrightness"), brightness);

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
    glUniform1f(glGetUniformLocation(shaderProgram, "uBrightness"), 0.f);
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
/*void Render::InitTextRendering() {
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

    //to myself: (gonna prob put in a cache or smth similar to image for rubrics (support multiple font types)
    if (FT_New_Face(ft, (FilePath::ASSETS_FOLDER + "PermanentMarker-Regular.ttf").c_str(), 0, &face)) { //for now, direct path is placed in here, danny's font
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //GL_NEAREST if it appears blurry
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
}*/

void Render::InitTextRendering() {
    // recreate VAO/VBO safely
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

bool Render::LoadFont(const std::string& fontPath, int sizePx, const std::string& key) {
    // If already loaded, do nothing
    if (fontCache.find(key) != fontCache.end()) {
        return true;
    }

    FT_Library ft = nullptr;
    FT_Face face = nullptr;

    auto cleanupFT = [&]() {
        if (face) { FT_Done_Face(face); face = nullptr; }
        if (ft) { FT_Done_FreeType(ft); ft = nullptr; }
        };

    if (FT_Init_FreeType(&ft)) {
        std::cerr << "FREETYPE: Could not init library\n";
        cleanupFT(); return false;
    }
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cerr << "FREETYPE: Failed to load font: " << fontPath << "\n";
        cleanupFT(); return false;
    }

    FT_Set_Pixel_Sizes(face, 0, static_cast<FT_UInt>(sizePx));
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // glyph bitmaps are byte-aligned

    Font font;
    font.sizePx = sizePx;
    font.name = key;

    for (unsigned char c = 0; c < 128; ++c) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "FREETYPE: Failed glyph " << (int)c << "\n";
            // make an empty glyph to avoid UB
            font.glyphs[c] = Character{ 0, {0,0}, {0,0}, 0 };
            continue;
        }

        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows, 0,
            GL_RED, GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // try GL_NEAREST if you want crisper pixels
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character ch{
            tex,
            { static_cast<int>(face->glyph->bitmap.width),
              static_cast<int>(face->glyph->bitmap.rows) },
            { face->glyph->bitmap_left, face->glyph->bitmap_top },
            static_cast<unsigned int>(face->glyph->advance.x >> 6) // *pixels*
        };
        font.glyphs[c] = ch;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    cleanupFT();

    fontCache.emplace(key, std::move(font));
    return true;
}

void Render::UnloadFont(const std::string& key) {
    auto it = fontCache.find(key);
    if (it == fontCache.end()) return;
    for (auto const& ch : it->second.glyphs) {
        if (ch.TextureID) {
            glDeleteTextures(1, &const_cast<unsigned int&>(ch.TextureID));
        }
    }
    fontCache.erase(it);
}

void Render::UnloadAllFonts() {
    for (auto& [k, f] : fontCache)
        for (auto const& ch : f.glyphs)
            if (ch.TextureID) {
                glDeleteTextures(1, &const_cast<unsigned int&>(ch.TextureID));
            }
    fontCache.clear();
}




/*void Render::DrawText(std::string text, float x, float y, float scale, const Colour3& color) {

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
}*/
void Render::DrawText(std::string text, float x, float y, float scale, const Colour3& color, const std::string& fontkey) {

    auto it = fontCache.find(fontkey);
    if (it == fontCache.end()) {
        // Font not found — fall back to first available font
        if (fontCache.empty()) {
            std::cerr << "This font type does not exists: " << fontkey << " (no fonts loaded)\n";
            return;
        }
        it = fontCache.begin();
    }

    const Font& font = it->second;

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
        //Character ch = Characters[*c];
        Character ch = font.glyphs[*c];

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
        //x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        x += (ch.Advance) * scale; //THIS IS WHERE I WENT WRONG, DONT NEED BITSHIFT AGAIN
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUniform1i(glGetUniformLocation(shaderProgram, "uUseText"), GL_FALSE); //Remember tell shader ok im done using text
}


/*void Render::DrawTextBox(const std::string& text, float x, float y, float rowWidth, float scale, const Colour3& color) {

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

}*/


void Render::DrawTextBox(const std::string& text, float x, float y, float rowWidth, float scale, const Colour3& color, const std::string& fontKey)
{
    auto it = fontCache.find(fontKey);
    if (it == fontCache.end()) {
        // Font not found — fall back to first available font
        if (fontCache.empty()) {
            std::cerr << "This font type does not exists: " << fontKey << " (no fonts loaded)\n";
            return;
        }
        it = fontCache.begin();
    }

    const Font& font = it->second;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(shaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3f(glGetUniformLocation(shaderProgram, "uTextColor"), color.r, color.g, color.b);
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseText"), GL_TRUE);
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), 0.0f, 0.0f);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), 0.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);

    float nextX = x;
    float nextY = y;
    float spaceWidth = static_cast<float>(font.glyphs[' '].Advance) * scale;
    float lineHeight = font.glyphs['H'].Size.y * 1.2f * scale;

    std::istringstream iss(text);
    std::string word;

    while (iss >> word) {
        // measure word width
        float wordWidth = 0.0f;
        for (char c : word) {
            const Character& ch = font.glyphs[c];
            wordWidth += static_cast<float>(ch.Advance) * scale;
        }

        // wrap if need, see if it will flow out the width of row
        if (nextX + wordWidth > x + rowWidth && nextX > x) {
            nextX = x;
            nextY -= lineHeight;
        }

        // render the each character
        for (char c : word) {
            const Character& ch = font.glyphs[c];

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
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            nextX += static_cast<float>(ch.Advance) * scale;
        }

        nextX += spaceWidth;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseText"), GL_FALSE);

}





//Load all image assets from JSON file (containing the image paths)
std::unordered_map<std::string, GLuint> Render::LoadImageAssetsFromJSON(const std::string& jsonPath) {

    //maybe can default create a json for image if doesnt exists?
    std::unordered_map<std::string, GLuint> result;

    //read file
    std::ifstream file{ jsonPath };
    //check if the json file for images exists
    if (!file.is_open()) {
        std::cerr << "Failed to open Image Asset JSON: " << jsonPath << std::endl;
        return result;
    }

    json j;
    try {
        file >> j;
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing Image JSON file: " << e.what() << std::endl;
        return result;
    }

    //"images" is the name of the array in the json (if u edit the name in the file, need remember update here)
    if (!j.contains("images") || !j["images"].is_array()) {
        std::cerr << "Invalid JSON format: missing \"images\" array in " << jsonPath << std::endl;
        return result;
    }

    for (const auto& entry : j["images"]) {

        std::string path;
        std::string alias;

        if (entry.is_string()) {
            path = FilePath::Resolve(entry.get<std::string>());
            alias = path; // fallback use path as key if alias not provided
        }
        else if (entry.is_object()) {
            path = FilePath::Resolve(entry.value("path", ""));
            alias = entry.value("alias", path);
        }

        if (path.empty()) {
            continue;
        }

        if (!std::filesystem::exists(path)) {
            std::cerr << "Missing Image file: " << path << std::endl;
            continue;
        }

        GLuint texID = LoadTexture(path);

        if (texID != 0) {
            result[alias] = texID; //store in map
        }
    }

    //just checking if logic works
    std::cout << "[Render] Loaded " << result.size() << " textures from " << jsonPath << std::endl;

    return result;
}




//Load font assets from json file (contain the fonts type)
std::unordered_map<std::string, Font> Render::LoadFontAssetsFromJSON(const std::string& jsonPath) {
    std::unordered_map<std::string, Font> result;

    std::ifstream file{ jsonPath };
    if (!file.is_open()) {
        std::cerr << "Failed to open Font Asset JSON: " << jsonPath << std::endl;
        return result;
    }

    json j;
    try {

        file >> j;
    }

    catch (const std::exception& e) {
        std::cerr << "Error parsing Font JSON: " << e.what() << std::endl;
        return result;
    }

    if (!j.contains("fonts") || !j["fonts"].is_array()) {
        std::cerr << "Invalid JSON format: missing \"fonts\" array in " << jsonPath << std::endl;
        return result;
    }

    for (const auto& entry : j["fonts"]) {

        if (!entry.contains("name") || !entry.contains("path") || !entry.contains("size")) {
            std::cerr << "Invalid font entry in JSON (missing name/path/size)" << std::endl;
            continue;
        }

        std::string fontName = entry["name"].get<std::string>();
        std::string fontPath = FilePath::Resolve(entry["path"].get<std::string>());
        int fontSize = entry["size"].get<int>();

        if (LoadFont(fontPath, fontSize, fontName)) {
            std::cout << "Loaded font: " << fontName << " (" << fontSize << "px)" << std::endl;
            result[fontName] = fontCache[fontName];
        }
        else {
            std::cerr << "Failed to load font: " << fontName << std::endl;
        }
    }

    //just checking if logic works
    std::cout << "[Font] Loaded " << result.size() << " fonts from " << jsonPath << std::endl;

    return result;
}


//shader interface functions
void Render::ApplyShader(GLuint shaderID) {
    glUseProgram(shaderID);
}

void Render::ResetShader() {
    glUseProgram(0);
}

void Render::SetUniform1f(GLuint shaderID, const std::string& name, float value) {
    GLint loc = glGetUniformLocation(shaderID, name.c_str());
    if (loc != -1)
        glUniform1f(loc, value);
}

void Render::SetUniform2f(GLuint shaderID, const std::string& name, const glm::vec2& value) {
    GLint loc = glGetUniformLocation(shaderID, name.c_str());
    if (loc != -1)
        glUniform2f(loc, value.x, value.y);
}

void Render::SetUniform3f(GLuint shaderID, const std::string& name, const glm::vec3& vec) {
    GLint loc = glGetUniformLocation(shaderID, name.c_str());
    if (loc != -1)
        glUniform3f(loc, vec.x, vec.y, vec.z);
}

void Render::SetUniformMat4(GLuint shaderID, const std::string& name, const glm::mat4& mat) {
    GLint loc = glGetUniformLocation(shaderID, name.c_str());
    if (loc != -1)
        glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
}



//Batching stuff attempt=============================================================================================================================================================
void Render::InitBatch()
{
    //GPU buffers
    glGenVertexArrays(1, &batchVAO);
    glGenBuffers(1, &batchVBO);
    glGenBuffers(1, &batchEBO);

    glBindVertexArray(batchVAO);
    glBindBuffer(GL_ARRAY_BUFFER, batchVBO);
    glBufferData(GL_ARRAY_BUFFER,
        MaxVertices * sizeof(BatchVertex),
        nullptr,
        GL_DYNAMIC_DRAW);

    //build the index buffer for MaxQuads
    std::vector<unsigned int> indices;
    indices.reserve(MaxIndices);
    unsigned int offset = 0;

    for (size_t i = 0; i < MaxQuads; ++i) {
        // 2 triangles per quad: (0,1,2) (2,3,0)
        indices.push_back(offset + 0);
        indices.push_back(offset + 1);
        indices.push_back(offset + 2);
        indices.push_back(offset + 2);
        indices.push_back(offset + 3);
        indices.push_back(offset + 0);
        offset += 4;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(unsigned int),
        indices.data(),
        GL_STATIC_DRAW);

    // Vertex layout (as in the batch vertex shader)
    // layout(location=0) vec2 position
    // layout(location=1) vec2 texCoord
    // layout(location=2) vec4 color
    // layout(location=3) float texIndex
    GLsizei stride = static_cast<GLsizei>(sizeof(BatchVertex));
    std::size_t off = 0;

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)off);
    off += sizeof(glm::vec2);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)off);
    off += sizeof(glm::vec2);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)off);
    off += sizeof(glm::vec4);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void*)off);

    glBindVertexArray(0);

    //Shader program for batches — loaded in LoadShaders(), skip here
    batchShader = 0;

    //CPU buffer reserve mem
    batchCPUBuffer.reserve(MaxVertices);

    //Sampler array binding, telling GLSL that uTextures[i] is the texture unit i
    glUseProgram(batchShader);
    GLint loc = glGetUniformLocation(batchShader, "uTextures");
    int samplers[MaxTexSlots];

    for (int i = 0; i < static_cast<int>(MaxTexSlots); ++i) {
        samplers[i] = i;
    }

    if (loc >= 0) {
        glUniform1iv(loc, static_cast<GLsizei>(MaxTexSlots), samplers);
    }

    // Also set the projection once here, update projection elsewhere if needed
    /*GLint projLoc = glGetUniformLocation(batchShader, "uProjection");
    if (projLoc >= 0) {
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
    } // reuse existing projection (see first if need use camera view project matrix)*/

    //use camera view proj matrix
    glm::mat4 projMatrix = useCamera
        ? mainCamera.GetViewProjection()
        : projection;
    glUniformMatrix4fv(glGetUniformLocation(batchShader, "uProjection"),
        1, GL_FALSE, &projMatrix[0][0]);

    glUseProgram(0);

    ResetBatch();
}


void Render::ResetBatch()
{
    //reset everything
    batchCPUBuffer.clear();
    batchIndexCount = 0;
    batchTexSlots.clear();
    batchTexSlots.reserve(MaxTexSlots);
}

void Render::BeginBatch()
{
    ResetBatch();
}

static inline glm::vec2 Rotate2D(const glm::vec2& p, float radians)
{
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    return glm::vec2(c * p.x - s * p.y, s * p.x + c * p.y);
}

void Render::FlushIfFull(size_t quadCountNeeded, GLuint textureID)
{
    // If exceeding quad capacity, or no room for a new texture slot, flush.
    bool needFlush = false;

    if (batchIndexCount + quadCountNeeded * 6 > MaxIndices) {
        needFlush = true;
    }
    else {
        // Check if textureID is already in slots
        bool found = false;
        for (size_t i = 0; i < batchTexSlots.size(); ++i) {
            if (batchTexSlots[i] == textureID) {
                found = true;
                break;
            }
        }
        if (!found && batchTexSlots.size() >= MaxTexSlots) {
            needFlush = true;
        }
    }

    if (needFlush) {
        EndBatch();
        FlushBatch();
        ResetBatch();
    }
}

void Render::SubmitSpriteBatch(const Image& image, const Vec2& pos, const Vec2& size, float rotation)
{
    // full texture
    SubmitSpriteBatchUV(image, pos, size, glm::vec4(0.f, 1.f, 1.f, 0.f), rotation);

    /*std::cout << "SpriteBatch: texID=" << image.textureID
        << ", size=(" << size.x << "," << size.y << ")"
        << ", pos=(" << pos.x << "," << pos.y << ")"
        << std::endl;*/

}

void Render::SubmitSpriteBatchUV(const Image& image, const Vec2& pos, const Vec2& size, const glm::vec4& uvRect, float rotation)
{
    if (!image.IsValid()) {
        return;
    }

    // Ensure capacity / texture slot availability
    FlushIfFull(1, image.textureID);

    // Determine tex slot index
    float texIndex = -1.0f;
    for (size_t i = 0; i < batchTexSlots.size(); ++i) {
        if (batchTexSlots[i] == image.textureID) { texIndex = static_cast<float>(i); break; }
    }
    if (texIndex < 0.0f) {
        batchTexSlots.push_back(image.textureID);
        texIndex = static_cast<float>(batchTexSlots.size() - 1); // 0..N-1
    }

    // Build a quad centered at (0,0), then rotate/translate to pos
    // size = (w,h)
    const glm::vec2 half = { size.x * 0.5f, size.y * 0.5f};

    // CCW: bl, br, tr, tl  (match index pattern 0-1-2, 2-3-0)
    glm::vec2 local[4] = {
        {-half.x, -half.y}, // 0
        {+half.x, -half.y}, // 1
        {+half.x, +half.y}, // 2
        {-half.x, +half.y}  // 3
    };

    // Rotate then translate to world
    glm::vec2 world[4];
    for (int i = 0; i < 4; ++i) {
        world[i] = Rotate2D(local[i], rotation) + glm::vec2(pos.x, pos.y);
    }

    // UVs (u0,v0,u1,v1) => (bl, br, tr, tl)
    const float u0 = uvRect.x, v0 = uvRect.y, u1 = uvRect.z, v1 = uvRect.w;

    glm::vec2 uvs[4] = {
        { u0, v1 }, // bl
        { u1, v1 }, // br
        { u1, v0 }, // tr
        { u0, v0 }  // tl
    };

    // Push 4 vertices
    BatchVertex v;
    v.color = kWhite;
    v.texIndex = texIndex;

    for (int i = 0; i < 4; ++i) {
        v.position = world[i];
        v.texCoord = uvs[i];
        batchCPUBuffer.push_back(v);
    }

    batchIndexCount += 6; // one more quad (two triangles)
}

void Render::EndBatch()
{
    if (batchCPUBuffer.empty()) {
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, batchVBO);
    glBufferSubData(GL_ARRAY_BUFFER,
        0,
        static_cast<GLsizeiptr>(batchCPUBuffer.size() * sizeof(BatchVertex)),
        batchCPUBuffer.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Render::FlushBatch()
{
    if (batchCPUBuffer.empty() || batchIndexCount == 0) {
        return;
    }

    // Bind textures to slots 0..till N-1
    for (size_t i = 0; i < batchTexSlots.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(i));
        glBindTexture(GL_TEXTURE_2D, batchTexSlots[i]);
    }

    glUseProgram(batchShader);

    // Ensure projection is current (
    // might need use camera view projection)
    /*GLint projLoc = glGetUniformLocation(batchShader, "uProjection");
    if (projLoc >= 0) {
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
    }*/

    //use camera view proj matrix
    glm::mat4 projMatrix = useCamera
        ? mainCamera.GetViewProjection()
        : projection;
    glUniformMatrix4fv(glGetUniformLocation(batchShader, "uProjection"),
        1, GL_FALSE, &projMatrix[0][0]);

    glBindVertexArray(batchVAO);
    glDrawElements(GL_TRIANGLES,
        static_cast<GLsizei>(batchIndexCount),
        GL_UNSIGNED_INT,
        nullptr);

    // Cleanup binds
    glBindVertexArray(0);
    glUseProgram(0);

    //batch testing debug
    /*std::cout << "[BatchRenderer] Flush: "
        << batchTexSlots.size() << " textures, "
        << batchIndexCount / 6 << " quads"
        << std::endl;*/
}

//batch attempt end =========================================================================================================================================================================
//void Render::SetTint(const Colour3& tint) {
//    currentTint = { tint.r, tint.g, tint.b };
//}
//void Render::ClearTint() {
//    currentTint = { 1.f, 1.f, 1.f };
//}

Vec2 Render::CalculateTextSize(const std::string& text, float scale, const std::string& fontKey)
{
    /*float width = 0.0f;
    float height = 0.0f;

    for (char c : text)
    {
        if (Characters.find(c) == Characters.end())
            continue;

        const Character& ch = Characters[c];
        width += (ch.Advance >> 6) * scale; // Advance in pixels
        height = std::max(height, (float)ch.Size.y * scale);
    }

    return { width, height };*/

    Vec2 TextDimensions{ 0.0f, 0.0f };

    //check if font exists
    auto it = fontCache.find(fontKey);
    if (it == fontCache.end()) {
        // Font not found — fall back to first available font
        if (fontCache.empty()) {
            std::cerr << "CalculateTextSize: font not found: " << fontKey << " (no fonts loaded)\n";
            return TextDimensions;
        }
        it = fontCache.begin();
    }
    const Font& font = it->second;

    for (char c : text)
    {
        // ASCII only (0..127); skip if out of range
        unsigned char uc = static_cast<unsigned char>(c);
        const Character& ch = font.glyphs[uc];

        // WIDTH. accumulate width by glyph advance (already in pixels), no need bit shift again, alrdy done in loadfont
        TextDimensions.x += static_cast<float>(ch.Advance) * scale;

        // HEIGHT. tallest glyph height seen so far
        float h = static_cast<float>(ch.Size.y) * scale;
        TextDimensions.y = (h > TextDimensions.y) ? h : TextDimensions.y; // 
    }

    return TextDimensions;
}


void Render::DrawRoundedRect(const Vec2& pos, float width, float height, float radius, const Colour3& color) {
    // base rectangle
    DrawRect(pos, width - 2 * radius, height, color);
    DrawRect(pos, width, height - 2 * radius, color);

    // 4 quarter-circles for corners
    Vec2 corners[] = {
        { pos.x - width / 2 + radius, pos.y + height / 2 - radius }, // top-left
        { pos.x + width / 2 - radius, pos.y + height / 2 - radius }, // top-right
        { pos.x - width / 2 + radius, pos.y - height / 2 + radius }, // bottom-left
        { pos.x + width / 2 - radius, pos.y - height / 2 + radius }  // bottom-right
    };
    for (auto& c : corners)
        DrawCircle(c, radius, color, 16);
}
void Render::DrawFullscreenTexture(GLuint texture, bool cover)
{
    if (!texture) return;

    // 1. Get texture dimensions
    int texW = 0, texH = 0;
    glBindTexture(GL_TEXTURE_2D, texture);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texW);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texH);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (texW == 0 || texH == 0)
        return; // invalid texture, skip draw

    // 2. Compute scaling that maintains aspect ratio 
    //okay gotta figure this out, cause now i set it to always use a virtual 1920x1080 resolution so can match it when in full screen mode
    // but gotta think about the case, whether it uses windowed or full screen???
    float winW, winH;
    if (resMode == ResolutionMode::Virtual) {
        winW = 1920.0f;
        winH = 1080.0f;
    }
    else {
        winW = static_cast<float>(GLApp::width);
        winH = static_cast<float>(GLApp::height);
    }
    //const float winW = 1920.f;//static_cast<float>(GLApp::width);
    //const float winH = 1080.f;//static_cast<float>(GLApp::height);
    const float wr = winW / texW;
    const float hr = winH / texH;

    // "cover" fills & crops, "contain" fits & letterboxes
    const float s = cover ? std::max(wr, hr) : std::min(wr, hr);
    const Vec2 drawSize = { texW * s, texH * s };

    // 3. Center and draw
    DrawTexturedQuad({ winW * 0.5f, winH * 0.5f }, drawSize, texture);
}

/*void Render::DrawFullscreenTexture(GLuint texture, bool cover)
{
    if (!texture) return;

    //Get texture dimensions
    int texW = 0, texH = 0;
    glBindTexture(GL_TEXTURE_2D, texture);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texW);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texH);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (texW == 0 || texH == 0)
        return; // invalid texture, skip draw

    //Use actual window size for main menu / backgrounds
    const float winW = static_cast<float>(GLApp::width);
    const float winH = static_cast<float>(GLApp::height);

    const float wr = winW / texW;
    const float hr = winH / texH;

    // "cover" = fill and crop, "contain" = fit and letterbox
    const float s = cover ? std::max(wr, hr) : std::min(wr, hr);

    const float drawW = texW * s;
    const float drawH = texH * s;

    const float posX = (winW - drawW) * 0.5f;
    const float posY = (winH - drawH) * 0.5f;

    DrawScreenQuad(texture, posX, posY, drawW, drawH);
}*/

void Render::UseVirtualWindow(bool enable) {
    resMode = enable ? ResolutionMode::Virtual : ResolutionMode::Window;
}

void Render::InitSceneFramebuffer(int width, int height)
{
    if (sceneFBO) {
        glDeleteFramebuffers(1, &sceneFBO);
        glDeleteTextures(1, &sceneColor);
        glDeleteRenderbuffers(1, &sceneDepth);
    }

    glGenFramebuffers(1, &sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

    // Create color texture
    glGenTextures(1, &sceneColor);
    glBindTexture(GL_TEXTURE_2D, sceneColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, sceneColor, 0);

    // Depth buffer
    glGenRenderbuffers(1, &sceneDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, sceneDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER, sceneDepth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "[Render] Scene framebuffer incomplete!\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Render::BeginSceneCapture()
{
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
    glViewport(0, 0, GLApp::width, GLApp::height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Render::EndSceneCapture()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



static const char* kDefaultVert = R"GLSL(
#version 450 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;
uniform mat4 uProjection;
uniform vec2 uPos;
uniform vec2 uScale;
uniform float uRotation;
uniform vec4 uUVRect;
uniform bool uUseSubImage;
uniform bool uUseCustomVerts;
out vec2 TexCoord;
void main() {
    vec2 world;
    if (uUseCustomVerts) {
        world = aPos;
    } else {
        float c = cos(uRotation); float s = sin(uRotation);
        mat2 rot = mat2(c, -s, s, c);
        world = rot * aPos * uScale + uPos;
    }
    gl_Position = uProjection * vec4(world, 0.0, 1.0);
    TexCoord = uUseSubImage ? mix(uUVRect.xy, uUVRect.zw, aTex) : aTex;
}
)GLSL";

static const char* kDefaultFrag = R"GLSL(
#version 450 core
out vec4 FragColor;
uniform sampler2D uTexture;
uniform bool uUseTexture;
uniform vec3 uColor;
uniform bool uUseText;
uniform vec3 uTextColor;
uniform float uAlpha;
uniform float uBrightness;
in vec2 TexCoord;
void main() {
    if (uUseText) {
        float alpha = texture(uTexture, TexCoord).r;
        FragColor = vec4(uTextColor, alpha);
    } else if (uUseTexture) {
        vec4 t = texture(uTexture, TexCoord);
        vec3 tinted = mix(t.rgb * (uColor * 0.5 + 0.5), vec3(1.0), uBrightness);
        FragColor = vec4(tinted, t.a);
    } else {
        FragColor = vec4(uColor, uAlpha);
    }
}
)GLSL";

static const char* kDefaultBatchVert = R"GLSL(
#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;
layout(location = 2) in vec4 aColor;
layout(location = 3) in float aTexIndex;
uniform mat4 uProjection;
out vec2 vTex;
out vec4 vColor;
flat out int vTexIndex;
void main() {
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
    vTex = aTex; vColor = aColor; vTexIndex = int(aTexIndex);
}
)GLSL";

static const char* kDefaultBatchFrag = R"GLSL(
#version 450 core
in vec2 vTex;
in vec4 vColor;
flat in int vTexIndex;
out vec4 FragColor;
uniform sampler2D uTextures[16];
void main() {
    FragColor = texture(uTextures[vTexIndex], vTex) * vColor;
}
)GLSL";

void Render::LoadShaders()
{
    if (FilePath::ASSETS_FOLDER.empty()) return;

    namespace fs = std::filesystem;
    std::string vertPath  = FilePath::ASSETS_FOLDER + "vertexSrc.vert";
    std::string fragPath  = FilePath::ASSETS_FOLDER + "fragmentSrc.frag";
    std::string bvertPath = FilePath::ASSETS_FOLDER + "batch.vert";
    std::string bfragPath = FilePath::ASSETS_FOLDER + "batch.frag";

    // Seed missing shader files with defaults so new games render immediately
    auto seedShader = [](const std::string& dst, const char* src) {
        if (!std::filesystem::exists(dst)) { std::ofstream o(dst); if (o) o << src; }
    };
    seedShader(vertPath,  kDefaultVert);
    seedShader(fragPath,  kDefaultFrag);
    seedShader(bvertPath, kDefaultBatchVert);
    seedShader(bfragPath, kDefaultBatchFrag);

    if (shaderProgram) { glDeleteProgram(shaderProgram); shaderProgram = 0; }
    if (fs::exists(vertPath) && fs::exists(fragPath))
        shaderProgram = CreateProgramFromFiles(vertPath, fragPath);

    if (batchShader) { glDeleteProgram(batchShader); batchShader = 0; }
    if (fs::exists(bvertPath) && fs::exists(bfragPath))
        batchShader = CreateProgramFromFiles(bvertPath, bfragPath);

    // Re-bind sampler array for batch shader
    if (batchShader)
    {
        glUseProgram(batchShader);
        GLint loc = glGetUniformLocation(batchShader, "uTextures");
        if (loc != -1)
        {
            int samplers[16];
            for (int i = 0; i < 16; ++i) samplers[i] = i;
            glUniform1iv(loc, 16, samplers);
        }
        glUseProgram(0);
    }

    // Reload fonts now that ASSETS_FOLDER is valid
    InitTextRendering();
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

    std::cout << "[Render] Shutdown complete, all GPU resources freed.\n";
}

Vec2 Render::ScreenToWorld(float ndcX, float ndcY) const {
    Camera& cam = Camera::GetInstance();
    glm::mat4 invVP = glm::inverse(cam.GetViewProjection());
    glm::vec4 world = invVP * glm::vec4(ndcX, ndcY, 0.0f, 1.0f);
    if (world.w != 0.0f) world /= world.w;
    return { world.x, world.y };
}


Vec2 Render::PanelToWorld(const ImVec2& mouse, const ImVec2& panelPos, const ImVec2& panelSize)
{
    Camera& cam = Camera::GetInstance();
    Vec2 camPos = cam.GetPosition();
    float zoom = cam.GetZoom();

    float u = (mouse.x - panelPos.x) / panelSize.x;
    float v = 1.f - ((mouse.y - panelPos.y) / panelSize.y);

    float ndcX = u * 2.f - 1.f;
    float ndcY = v * 2.f - 1.f;

    float halfWidth = (GLApp::width * 0.5f) / zoom;
    float halfHeight = (GLApp::height * 0.5f) / zoom;
    
    return {
        camPos.x + ndcX * halfWidth,
        camPos.y + ndcY * halfHeight
    };
}

ImVec2 Render::WorldToPanel(const Vec2& world, const ImVec2& panelPos, const ImVec2& panelSize)
{
    // Use the camera's actual view-projection matrix so the outline matches
    // the rendered scene exactly, regardless of resolution or fullscreen mode.
    Camera& cam = Camera::GetInstance();
    Vec2 ndc = cam.WorldToNDC(world);

    float u = ndc.x * 0.5f + 0.5f;
    float v = 1.f - (ndc.y * 0.5f + 0.5f);

    return ImVec2(
        panelPos.x + u * panelSize.x,
        panelPos.y + v * panelSize.y
    );
}

void Render::SetWindowScale(float scaleX, float scaleY) {
    m_WindowScale = { scaleX, scaleY };
}

void Render::PresentScene()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    float targetW = (float)GLApp::VIRTUAL_W;   // 1920
    float targetH = (float)GLApp::VIRTUAL_H;   // 1080
    float winW = (float)GLApp::width;
    float winH = (float)GLApp::height;

    float scale = std::min(winW / targetW, winH / targetH);

    int drawW = (int)(targetW * scale);
    int drawH = (int)(targetH * scale);

    int offsetX = (int)((winW - drawW) * 0.5f);
    int offsetY = (int)((winH - drawH) * 0.5f);

    // Clear screen to black (letterbox bars)
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set viewport to the letterboxed region
    glViewport(offsetX, offsetY, drawW, drawH);

    // Draw the scene texture using your existing fullscreen quad
    DrawTexturedQuad({ 0,0 }, { targetW,targetH }, GetSceneTexture());
}


void Render::DrawLetterboxedTexture(GLuint texture)
{
    if (!texture) return;

    //Get texture size
    int texW = 0, texH = 0;
    glBindTexture(GL_TEXTURE_2D, texture);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texW);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texH);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (texW == 0 || texH == 0)
        return;

    //Compute letterboxed region in pixels
    float winW = static_cast<float>(GLApp::width);
    float winH = static_cast<float>(GLApp::height);

    float texAspect = static_cast<float>(texW) / static_cast<float>(texH);
    float winAspect = winW / winH;

    float drawW, drawH;

    if (winAspect > texAspect)
    {
        // Window is wider than texture, fit HEIGHT, add bars left/right
        drawH = winH;
        drawW = drawH * texAspect;
    }
    else
    {
        // Window is talle, fit WIDTH, add bars top/bottom
        drawW = winW;
        drawH = drawW / texAspect;
    }

    float offsetX = (winW - drawW) * 0.5f;
    float offsetY = (winH - drawH) * 0.5f;

    //screen-space projection (no camera) ---
    bool oldUseCam = useCamera;
    useCamera = false;

    // Temporarily use actual screen projection for the background quad;
    // save and restore so subsequent entity rendering uses whatever projection set before.
    glm::mat4 savedProjection = projection;
    projection = glm::ortho(0.f, winW, 0.f, winH, -1.f, 1.f);

    // Center of the letterbox rect in screen space
    Vec2 center{
        offsetX + drawW * 0.5f,
        offsetY + drawH * 0.5f
    };

    Vec2 size{ drawW, drawH };

    // Just draw the quad in the computed rect
    DrawTexturedQuad(center, size, texture);

    // Restore projection and camera flag
    projection = savedProjection;
    useCamera = oldUseCam;
}

/*void Render::DrawLetterboxedTexture(GLuint texture)
{
    if (!texture) return;

    // --- 1. Get texture size ---
    int texW = 0, texH = 0;
    glBindTexture(GL_TEXTURE_2D, texture);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texW);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texH);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (texW == 0 || texH == 0)
        return;

    // --- 2. Use whatever viewport is active ---
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);   // <- IMPORTANT
    float vpX = vp[0];
    float vpY = vp[1];
    float vpW = vp[2];
    float vpH = vp[3];

    // --- 3. Compute aspect fit *inside viewport*
    float texAspect = (float)texW / (float)texH;
    float vpAspect = vpW / vpH;

    float drawW, drawH;

    if (vpAspect > texAspect)
    {
        drawH = vpH;
        drawW = drawH * texAspect;
    }
    else
    {
        drawW = vpW;
        drawH = drawW / texAspect;
    }

    // --- 4. Build projection JUST for current viewport ---
    glm::mat4 proj = glm::ortho(0.0f, drawW, 0.0f, drawH);

    // disable camera
    bool oldCam = useCamera;
    useCamera = false;

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &proj[0][0]);

    // --- 5. Draw quad centered in viewport ---
    Vec2 center{ drawW * 0.5f, drawH * 0.5f };
    Vec2 size{ drawW, drawH };

    DrawTexturedQuad(center, size, texture);

    useCamera = oldCam;
}*/


void Render::ApplyLetterboxViewport()
{
    float targetAspect = 16.0f / 9.0f;
    float windowAspect = (float)GLApp::width / (float)GLApp::height;

    int vpX = 0, vpY = 0;
    int vpW = GLApp::width;
    int vpH = GLApp::height;

    if (windowAspect > targetAspect)
    {
        // Window is too wide
        vpW = int(GLApp::height * targetAspect);
        vpX = (GLApp::width - vpW) / 2;
    }
    else
    {
        // Window is too tal
        vpH = int(GLApp::width / targetAspect);
        vpY = (GLApp::height - vpH) / 2;
    }

    glViewport(vpX, vpY, vpW, vpH);
}


/*void Render::DrawScreenQuad(GLuint texture, float x, float y, float w, float h)
{
    if (!texture) return;

    // Save state we’re about to override
    bool prevUseCamera = useCamera;
    glm::mat4 prevProjection = projection;

    // We want *pure screen-space*, no camera
    useCamera = false;

    // Projection in pixels: (0,0) bottom-left, (width,height) top-right
    projection = glm::ortho(
        0.0f, static_cast<float>(GLApp::width),
        0.0f, static_cast<float>(GLApp::height),
        -1.0f, 1.0f
    );

    // Center position for your existing DrawTexturedQuad API
    Vec2 center{ x + w * 0.5f, y + h * 0.5f };
    Vec2 size{ w, h };

    // This will now use the updated `projection` and `useCamera = false`
    DrawTexturedQuad(center, size, texture);

    // Restore previous state so game/world rendering isn’t affected
    useCamera = prevUseCamera;
    projection = prevProjection;
}

void Render::PresentScene()
{
    GLuint tex = GetSceneTexture();
    if (!tex) return;

    float virtualW = 1920.f;
    float virtualH = 1080.f;

    float winW = (float)GLApp::width;
    float winH = (float)GLApp::height;

    float sx = winW / virtualW;
    float sy = winH / virtualH;
    float s = std::min(sx, sy);

    float renderW = virtualW * s;
    float renderH = virtualH * s;

    float offsetX = (winW - renderW) * 0.5f;
    float offsetY = (winH - renderH) * 0.5f;

    // Clear real window
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, winW, winH);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the scaled, centered scene in screen space
    DrawScreenQuad(tex, offsetX, offsetY, renderW, renderH);
}*/