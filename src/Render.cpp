#include "Render.h"
#include "glapp.h"
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
}

void Render::ClearBackground(const Colour4& color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

//draw stuff
void Render::DrawSquare(const glm::vec2& pos, float size, const glm::vec3& color) {
    glUseProgram(shaderProgram);

    // upload projection
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projection[0][0]);

    // upload pos, size (use size for both X and Y), color
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), pos.x, pos.y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), size, size);
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

void Render::DrawCircle(const Vec2& center, float radius, const Colour3& color, int segments) {
    // vertices for a circle
    std::vector<GLfloat> vertices;
    vertices.reserve((segments + 2) * 2);

    //ctr point
    vertices.push_back(center.x);
    vertices.push_back(center.y);


    for (int i = 0; i <= segments; i++) {
        float angle = (2.0f * 3.1415 * i) / static_cast<float>(segments);
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
}

void Render::DrawRect(const Vec2& pos, float width, float height, const Colour3& color) {
    glUseProgram(shaderProgram);

    // upload projection
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"),
        1, GL_FALSE, &projection[0][0]);

    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), pos.x, pos.y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), width, height);
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), color.r, color.g, color.b);

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
        glDeleteTextures(1, &it->second);
        textureCache.erase(it);
    }
}

void Render::UnloadTexture(GLuint textureID) {
    // Find the texture in the cache
    for (auto it = textureCache.begin(); it != textureCache.end(); ++it) {
        if (it->second == textureID) {
            std::cout << "Unloading texture ID: " << textureID << " from path: " << it->first << std::endl;
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



bool Render::IsAreaTouched(const glm::vec2& mousePos, const glm::vec2& areaPos, float width, float height)
{
    glUniform2f(glGetUniformLocation(shaderProgram, "uPos"), areaPos.x, areaPos.y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), width, height);
    const glm::vec2 deltaPos = glm::abs(mousePos - areaPos);
    const float hx = 0.5f * std::abs(width);
    const float hy = 0.5f * std::abs(height);
    return (deltaPos.x <= hx) && (deltaPos.y <= hy);
}



//Image stuff
Image Render::LoadImage(const std::string& path) {
    // Check if already in image cache
    auto it = imageCache.find(path);
    if (it != imageCache.end()) {
        std::cout << "Image found in cache: " << path << std::endl;
        return it->second;
    }

    // Load texture using existing method
    GLuint textureID = LoadTexture(path);
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

    Image image = CreateImageFromTexture(textureID, width, height, path);
    imageCache[path] = image; //store in cache

    std::cout << "Loaded image: " << path << " (" << width << "x" << height << ")" << std::endl;
    return image;
}

void Render::DrawImage(const Image& image, float x, float y) {
    if (!image.IsValid()) {
        return;
    }

    Vec2 pos(x, y);
    Vec2 scale(static_cast<float>(image.width), static_cast<float>(image.height));
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

    float w = image.width * scaleX;
    float h = image.height * scaleY;
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
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), (float)image.width, (float)image.height);
    glUniform1f(glGetUniformLocation(shaderProgram, "uRotation"), angleRad);

    // bind texture + draw
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, image.textureID);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);   //arrays, not elements
    glBindVertexArray(0);

    glUseProgram(0);
}



int Render::GetImageWidth(const Image& image) const {
    return image.width;
}

int Render::GetImageHeight(const Image& image) const {
    return image.height;
}

void Render::UnloadImage(Image& image) {
    if (!image.IsValid()) {
        return;
    }

    // Remove from cache
    imageCache.erase(image.path);

    // Unload the texture
    UnloadTexture(image.textureID);

    // Invalidate the image
    image.textureID = 0;
    image.width = 0;
    image.height = 0;
    image.path.clear();

    std::cout << "Unloaded image: " << image.path << std::endl;
}

void Render::ClearAllImages() {
    for (auto& pair : imageCache) {
        UnloadTexture(pair.second.textureID);
    }
    imageCache.clear();
    std::cout << "All images cleared from cache" << std::endl;
}

Image Render::CreateImageFromTexture(GLuint textureID, int width, int height, const std::string& path) {
    return Image(textureID, width, height, path);
}
