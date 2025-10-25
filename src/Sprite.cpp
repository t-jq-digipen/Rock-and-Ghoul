/* ========================================================
 * @file	Sprite.cpp
 * @author	Tan Jun Quan | t.junquan@digipen.edu
 * @date	29 SEP 2025
 *
 * @brief	Sprite component for ECS
 *
 * This file declares the sprite component used for loading
 * and drawing images.
 *
 *
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "Sprite.h"

namespace Sprite {

	// Sprite Component

	SpriteComponent::SpriteComponent() : m_image(), m_scale{ 0.f,0.f }, m_anim{ false }, m_rot{ 0.f }, tint{ 0.0f, 0.0f, 0.0f } {}

	SpriteComponent::SpriteComponent(std::string const& path, Vec2 scale_offset, bool anim) : m_scale{ scale_offset }, m_anim{ anim }, m_rot{ 0.f } {
		m_image = CreateImage(path);
	}

	SpriteComponent::SpriteComponent(Image const& image, float const& width, float const& height, bool anim) :
		m_image(image),
		m_scale(width, height),
		m_anim(anim),
		m_rot{ 0.f }
	{}

	SpriteComponent::SpriteComponent(Image const& image, Vec2 const& dimensions, bool anim) :
		m_image(image),
		m_scale(dimensions),
		m_anim(anim),
		m_rot{ 0.f }
	{}

	void SpriteComponent::Set_Image(Image const& image)
	{
		m_image = image;
	}

	void SpriteComponent::Set_Size(float const& width, float const& height)
	{
		m_scale = { width,height };
	}

	void SpriteComponent::Set_Size(Vec2 const& dimensions)
	{
		m_scale = dimensions;
	}

	void SpriteComponent::Set_Animated(bool set)
	{
		m_anim = set;
	}

	const Image& SpriteComponent::Get_Image() const
	{
		return m_image;
	}

	const Vec2& SpriteComponent::Get_Scale() const
	{
		return m_scale;
	}

	bool SpriteComponent::IsAnimated() const
	{ return m_anim; }

	std::string SpriteComponent::Get_TexturePath() const
	{ return Render::GetInstance().Get_TexturePath(m_image.textureID); }


	void SpriteComponent::Set_Rotation(float rot) {
		m_rot = rot;
	}

	float SpriteComponent::Get_Rotation() const {
		return m_rot;
	}

	void SpriteComponent::Rotate(float deltaRot) {
		m_rot += deltaRot;

	}

	std::string SpriteComponent::Serialize() const {

		json j = {
			{"texturePath", Get_TexturePath()},
			{"scaleX", m_scale.x},
			{"scaleY", m_scale.y},
			{"animated", m_anim}

		};
		return j.dump();
	}

	SpriteComponent SpriteComponent::DeSerialize(const std::string& data) {
		json j = json::parse(data);

		return SpriteComponent(j["texturePath"], { j["scaleX"], j["scaleY"] }, j["animated"]);
	}
}