#pragma once
#include <string>
#include <vector>
#include <map>
#include <GL/glew.h> 

#include "rectpack2D/finders_interface.h"
#include "stb_image.h"

//Info about one image in the atlas (like the 1 frame of the sprite etc)
//It stores the info of where in the big texture its located, through the normalized UVs
struct SimpleSprite
{
    float u0, v0;
    float u1, v1;
    int width, height;
};


//The big texture info 
struct SimpleAtlas
{
    GLuint textureID = 0; //opengl texture id to that big texture
    int width = 0; //The width u give to that texture
    int height = 0; //The height u give to that texture
    std::map<std::string, SimpleSprite> sprites; //a map to lookup the images/sprites in the big texture
};

class SimpleAtlasBuilder
{
public:
    //prepare and loading image , before packing them
    bool addImage(const std::string& key, const std::string& file);
    bool build(SimpleAtlas& atlas, int maxSide = 2048);

private:
    //stores a loaded image waiting to be packed into the big texture
    struct ImageData
    {
        std::string key;
        int w, h;
        std::vector<unsigned char> pixels; // RGBA
    };

    std::vector<ImageData> images;
};
