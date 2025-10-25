#pragma once
#include "DataDefs.h"
#include "Render.h"
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

namespace Sprite {
    class SpriteComponent {
    private:
        Image m_image;
        Vec2 m_scale;
        bool m_anim;
        float m_rot;

    public:
        SpriteComponent();
        SpriteComponent(std::string const& path, Vec2 scale_offset, bool anim = false);
        SpriteComponent(Image const& image, float const& width, float const& height, bool anim = false);
        SpriteComponent(Image const& image, Vec2 const& dimensions, bool anim = false);

        void Set_Image(Image const& image);
        void Set_Size(float const& width, float const& height);
        void Set_Size(Vec2 const& dimensions);
        void Set_Animated(bool set);
        void Set_Rotation(float rot);
        void Rotate(float deltaRot);

        const Image& Get_Image() const;
        const Vec2& Get_Scale() const;
        bool IsAnimated() const;
        std::string Get_TexturePath() const;
        float Get_Rotation() const;

        // Serialization methods
        std::string Serialize() const;
        static SpriteComponent DeSerialize(const std::string& data);
    };
}