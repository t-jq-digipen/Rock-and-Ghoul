#pragma once
#include <string>
#include <vector>
#include <map>
#include <GL/glew.h> // or glad

#include "rectpack2D/finders_interface.h"
#include "stb_image.h"

struct SimpleSprite
{
    float u0, v0;
    float u1, v1;
    int width, height;
};

struct SimpleAtlas
{
    GLuint textureID = 0;
    int width = 0;
    int height = 0;
    std::map<std::string, SimpleSprite> sprites;
};

class SimpleAtlasBuilder
{
public:
    bool addImage(const std::string& key, const std::string& file);
    bool build(SimpleAtlas& atlas, int maxSide = 2048);

private:
    struct ImageData
    {
        std::string key;
        int w, h;
        std::vector<unsigned char> pixels; // RGBA
    };

    std::vector<ImageData> images;
};
