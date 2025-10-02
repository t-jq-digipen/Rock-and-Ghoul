/* ========================================================
 * @file	Sprite.h
 * @author	Tan Jun Quan
 * @date	29 SEP 2025
 *
 * @brief	Sprite component for ECS
 *
 * This file declares the sprite component used for loading
 * and drawing images.
 * Component stores image data, scaling, rotation, and
 * whether it is a spritesheet or not.
 *
 *
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include "Render.h"

namespace Sprite {
	
	/*!
	 * @class SpriteComponent
	 * @brief Sprite component for an entity.
	 *
	 * Encapsulates image data, scaling, rotation.
	 * Can represent both static images and animated spritesheets using bool m_anim.
	 * Animated spritesheets would refer to Animator component.
	 */
	class SpriteComponent {
	private:
		Image	m_image;	// Image struct
		Vec2	m_scale;	// Independant image scaling
		bool	m_anim;		// Bool if sprite is animated or static
		float	m_rot;		// Rotation value of image in degrees
	public:
		/**
		 * @brief Default constructor. 
		 * Initializes empty image with zero scale and zero rotation, with no animation.
		 */
		SpriteComponent();
		/**
		 * @brief Construct a sprite from an image file path.
		 *
		 * Loads an image from a file path and applies the specified scale and animation state.
		 *
		 * @param path Path to the image file to load.
		 * @param scale_offset Scaling factor applied to the sprite.
		 * @param anim Whether the sprite should be treated as animated (default = false).
		 */
		SpriteComponent(std::string const& path, Vec2 scale_offset, bool anim = false);
		/**
		 * @brief Construct a sprite from an existing image with width/height scale.
		 *
		 * @param image Reference to an existing Image object.
		 * @param width Width scaling factor.
		 * @param height Height scaling factor.
		 * @param anim Whether the sprite should be treated as animated (default = false).
		 */
		SpriteComponent(Image const& image, float const& width, float const& height, bool anim = false);
		/**
		 * @brief Construct a sprite from an existing image with a 2D scale vector.
		 *
		 * @param image Reference to an existing Image object.
		 * @param dimensions Scaling factor as a 2D vector.
		 * @param anim Whether the sprite should be treated as animated (default = false).
		 */
		SpriteComponent(Image const& image, Vec2 const& dimensions, bool anim = false);

		/**
		 * @brief Set the sprite’s image.
		 * @param image New image to assign.
		 */
		void Set_Image(Image const& image);
		/**
		 * @brief Set the sprite’s size using width and height values.
		 * @param width New width scaling factor.
		 * @param height New height scaling factor.
		 */
		void Set_Size(float const& width, float const& height);
		/**
		 * @brief Set the sprite’s size using a 2D vector.
		 * @param dimensions New scaling vector.
		 */
		void Set_Size(Vec2 const& dimensions);
		/**
		 * @brief Set whether the sprite is animated.
		 * @param set True to mark as animated, false for static.
		 */
		void Set_Animated(bool set);

		/**
		 * @brief Get the sprite’s image.
		 * @return Constant reference to the sprite’s Image.
		 */
		const Image& Get_Image() const;
		/**
		 * @brief Get the sprite’s scaling factor.
		 * @return Constant reference to the scaling vector.
		 */
		const Vec2& Get_Scale() const;
		/**
		 * @brief Check if the sprite is animated. (A spritesheet)
		 * @return True if animated, false otherwise.
		 */
		bool IsAnimated();
		
		/**
		 * @brief Get the file path of the texture associated with this sprite.
		 * @return String representing the texture’s file path.
		 */
		std::string Get_TexturePath();

		/**
		 * @brief Set the rotation value of the sprite.
		 * @param rot Rotation amount in degrees.
		 */
		void Set_Rotation(float rot);

		/**
		 * @brief Set the rotation value of the sprite.
		 * @param rot Rotation amount in degrees.
		 */
		float Get_Rotation() const;
		void Rotate(float deltaRot);
	};

}