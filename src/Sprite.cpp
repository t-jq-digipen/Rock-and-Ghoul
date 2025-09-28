/* ========================================================
 * @file	Sprite.cpp
 * @author	Tan Jun Quan
 * @date	29 SEP 2025
 *
 * @brief	Sprite component for ECS
 *
 * Sprite things (BRIEF TO BE DONE)
 *
 *
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "Sprite.h"

namespace Sprite {

	// Sprite Component

	SpriteComponent::SpriteComponent() : m_image(), m_dim{0.f,0.f}, m_anim{false} {}

	SpriteComponent::SpriteComponent(Image const& image, float const& width, float const& height, bool anim) :
		m_image(image), 
		m_dim(width, height),
		m_anim(anim) 
	{}

	SpriteComponent::SpriteComponent(Image const& image, Vec2 const& dimensions, bool anim) :
		m_image(image),
		m_dim(dimensions),
		m_anim(anim)
	{}

	void SpriteComponent::Set_Image(Image const& image)
	{ m_image = image; }

	void SpriteComponent::Set_Size(float const& width, float const& height)
	{ m_dim = {width,height}; }

	void SpriteComponent::Set_Size(Vec2 const& dimensions)
	{ m_dim=dimensions; }

	void SpriteComponent::Set_Animated(bool set)
	{ m_anim = set; }

	const Image& SpriteComponent::Get_Image() const
	{ return m_image; }

	const Vec2& SpriteComponent::Get_Dimensions() const
	{ return m_dim; }

	bool SpriteComponent::IsAnimated()
	{ return m_anim; }


}