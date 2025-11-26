#include "SimpleAtlas.h"
#include <algorithm> // for std::max
#include <cmath>

using namespace rectpack2D;

bool SimpleAtlasBuilder::addImage(const std::string& key, const std::string& file)
{
    int w = 0, h = 0, channels = 0;
    unsigned char* data = stbi_load(file.c_str(), &w, &h, &channels, 4);
    if (!data)
        return false;

    ImageData img;
    img.key = key;
    img.w = w;
    img.h = h;
    img.pixels.assign(data, data + (w * h * 4));

    stbi_image_free(data);

    images.push_back(img);
    return true;
}

bool SimpleAtlasBuilder::build(SimpleAtlas& atlas, int maxSide)
{
    if (images.empty())
        return false;

    using spaces_type = empty_spaces<true, default_empty_spaces>;
    using rect_type = output_rect_t<spaces_type>;

    std::vector<rect_type> rects;
    rects.reserve(images.size());

    long long totalArea = 0;
    for (const auto& img : images)
    {
        rects.emplace_back(0, 0, img.w, img.h, false);
        totalArea += img.w * img.h;
    }

    int guessSide = (int)std::sqrt((double)totalArea);
    guessSide = std::min(std::max(guessSide, 64), maxSide);

    int atlasSide = guessSide;

    std::optional<rect_wh> result;
    for (int attempt = 0; attempt < 8; attempt++)
    {
        bool fail = false;

        auto on_success = [&](rect_type&) { return callback_result::CONTINUE_PACKING; };
        auto on_fail = [&](rect_type&) { fail = true; return callback_result::ABORT_PACKING; };

        result = find_best_packing<spaces_type>(
            rects,
            make_finder_input(
                atlasSide,
                1,
                on_success,
                on_fail,
                flipping_option::DISABLED
            )
        );

        if (!result || fail)
        {
            atlasSide = std::min(atlasSide + atlasSide / 2, maxSide);
            continue;
        }

        break;
    }

    if (!result)
        return false;

    int atlasW = result->w;
    int atlasH = result->h;

    std::vector<unsigned char> pixels(atlasW * atlasH * 4, 0);

    for (size_t i = 0; i < images.size(); i++)
    {
        auto& img = images[i];
        auto& r = rects[i];

        for (int y = 0; y < img.h; y++)
        {
            const unsigned char* src = &img.pixels[(size_t)y * img.w * 4];
            unsigned char* dst = &pixels[((size_t)(r.y + y) * atlasW + r.x) * 4];

            std::copy(src, src + img.w * 4, dst);
        }
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
        atlasW, atlasH, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    atlas.textureID = tex;
    atlas.width = atlasW;
    atlas.height = atlasH;
    atlas.sprites.clear();

    for (size_t i = 0; i < images.size(); i++)
    {
        auto& img = images[i];
        auto& r = rects[i];

        SimpleSprite sp;
        sp.width = img.w;
        sp.height = img.h;
        sp.u0 = r.x / (float)atlasW;
        sp.v0 = r.y / (float)atlasH;
        sp.u1 = (r.x + r.w) / (float)atlasW;
        sp.v1 = (r.y + r.h) / (float)atlasH;

        atlas.sprites[img.key] = sp;
    }

    return true;
}
