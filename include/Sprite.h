/* ========================================================
 * @file	Sprite.h
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

#pragma once
#include "Render.h"

namespace Sprite {
	
	class SpriteComponent {
	private:
		Image	m_image;	// Image struct
		Vec2	m_dim;		// Dimensions (width,height)
		bool	m_anim;		// Bool if sprite is animated or static
	public:
		SpriteComponent();
		SpriteComponent(Image const& image, float const& width, float const& height, bool anim = false);
		SpriteComponent(Image const& image, Vec2 const& dimensions, bool anim = false);

		void Set_Image(Image const& image);
		void Set_Size(float const& width, float const& height);
		void Set_Size(Vec2 const& dimensions);
		void Set_Animated(bool set);

		const Image& Get_Image() const;
		const Vec2& Get_Dimensions() const;
		bool IsAnimated();
		
	};

}