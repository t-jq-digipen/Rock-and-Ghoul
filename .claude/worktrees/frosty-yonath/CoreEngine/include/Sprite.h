/* ========================================================
 * @file		Sprite.h
 * @author		Tan Jun Quan | t.junquan@digipen.edu
 * @date		26 OCT 2025
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
#include "DataDefs.h"
#include <nlohmann/json.hpp>
#include <string>

namespace Sprite {

	/*!
	 * @class SpriteComponent
	 * @brief Sprite component for an entity.
	 *
	 * Encapsulates image data, scaling, rotation.
	 * Can represent both static images and animated spritesheets using bool m_anim.
	 * Animated spritesheets would refer to Animator component.
	 */
	class SpriteComponent : public IRenderData {
	private:
		Image	m_image;	// Image struct
		GLuint  m_oldtexID;	// old texID to check updates
		Vec2	m_scale;	// Independant image scaling
		float	m_rot;		// Rotation value of image in degrees

		bool flipX = false;
		bool flipY = false;

		Colour3 tint;
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
		SpriteComponent(std::string const& path, Vec2 scale_offset);
		/**
		 * @brief Construct a sprite from an existing image with width/height scale.
		 *
		 * @param image Reference to an existing Image object.
		 * @param width Width scaling factor.
		 * @param height Height scaling factor.
		 * @param anim Whether the sprite should be treated as animated (default = false).
		 */
		SpriteComponent(Image const& image, float const& width, float const& height);
		/**
		 * @brief Construct a sprite from an existing image with a 2D scale vector.
		 *
		 * @param image Reference to an existing Image object.
		 * @param dimensions Scaling factor as a 2D vector.
		 * @param anim Whether the sprite should be treated as animated (default = false).
		 */
		SpriteComponent(Image const& image, Vec2 const& dimensions);

		/**
		 * @brief Set the sprite's image.
		 * @param image New image to assign.
		 */
		void Set_Image(Image const& image);
		/**
		 * @brief Set the sprite's size using width and height values.
		 * @param width New width scaling factor.
		 * @param height New height scaling factor.
		 */
		void Set_Size(float const& width, float const& height);
		/**
		 * @brief Set the sprite's size using a 2D vector.
		 * @param dimensions New scaling vector.
		 */
		void Set_Size(Vec2 const& dimensions);
		/**
		 * @brief Set whether the sprite is animated.
		 * @param set True to mark as animated, false for static.
		 */
		//void Set_Animated(bool set);

		/**
		 * @brief Get the sprite's image.
		 * @return Constant reference to the sprite�s Image.
		 */
		const Image& Get_Image() const;
		/**
		 * @brief Get the sprite's scaling factor.
		 * @return Constant reference to the scaling vector.
		 */
		const Vec2& Get_Scale() const;

		/**
		 * @brief Get the file path of the texture associated with this sprite.
		 * @return String representing the texture's file path.
		 */
		std::string Get_TexturePath() const;

		Vec2 Get_ImageSize() const;

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

		void Update_ImageSize();

		void SetFlipX(bool flip) { flipX = flip; }

		void SetFlipY(bool flip) { flipY = flip; }

		const bool GetFlipX() const { return flipX; }

		const bool GetFlipY() const { return flipY; }

		void Set_Tint(const Colour3& color) { tint = color; }
		const Colour3& Get_Tint() const { return tint; }

		void Update(float) override;

		RenderType GetType() const override { return RT_IMAGE; }

		std::unique_ptr<IRenderData> Clone() const override {
			return std::make_unique<SpriteComponent>(*this);
		}

		std::string Serialize() const override;

		static SpriteComponent DeSerialize(Entity eID, const std::string& data);

		Editor::ComponentData EditorData();
	};


	class SpriteSheet : public IRenderData {
		// Sprite data
	private:
		SpriteComponent m_sprite;

		// Animator data
	private:
		int m_total_rows;		// Total number of rows in spritesheet
		int m_total_cols;		// Total number of columns in spritesheet

		int m_current_row;		// Current row index of spritesheet
		int m_current_col;		// Current column index of spritesheet

		float m_uv_height;		// UV height of single frame
		float m_uv_width;		// UV width of single frame


		float anim_duration;	// Time to cycle through the frames in seconds
		float anim_timer;		// Time elapsed

		ActiveFrames frames;	// Which frames to call
		int range;				// Number of frames we are looping through

		// using int for key for enumeration
		std::unordered_map<unsigned int, ActiveFrames> stateframe;	// A map with int key for active frames

		// store state names
		std::unordered_map<std::string, unsigned int> stateNames;

		// Using int for enumeration compatibility with developers
		unsigned int m_current_state = 0;							// Animation state


	public:
		float u0, v0, u1, v1;								// For sub image drawing


	public:
		SpriteSheet();

		SpriteComponent& Sprite() { return m_sprite; }

		const SpriteComponent& Sprite() const { return m_sprite; }


		/**
		 * @brief Update UV coordinates based on the current row and column.
		 */
		void UpdateUV();

		void UpdateUV(int imageWidthPixels, int imageHeightPixels);

		// void UpdateStateFrames();

		/**
		 * @brief Set the active animation frames and duration.
		 * @param framerange The range of frames (row and column indices).
		 * @param duration Duration (in seconds) of the animation cycle. Zero means a single-frame animation.
		 * @throws std::runtime_error if duration == 0 but multiple frames are specified.
		 */
		void SetFrames(ActiveFrames const& framerange);

		/**
		 * @brief Reset the animation timer to zero.
		 */
		void ResetTimer();

		/**
		 * @brief Update the current animation frame based on elapsed time.
		 * @param dt Delta time (seconds) since the last update.
		 */
		void UpdateFrame(float dt);

		/**
		 * @brief Get the total number of rows in the sprite sheet.
		 * @return The number of rows.
		 */
		int GetTotalRows() const { return m_total_rows; }

		void SetTotalRows(int r) { m_total_rows = r; }

		/**
		 * @brief Get the total number of columns in the sprite sheet.
		 * @return The number of columns.
		 */
		int GetTotalCols() const { return m_total_cols; }

		void SetTotalCols(int c) { m_total_cols = c; }

		/**
		 * @brief Get the duration of the current animation cycle.
		 * @return The animation duration in seconds.
		 */
		float GetAnimDuration() const { return anim_duration; }

		/**
		 * @brief Get the currently active frame range.
		 * @return The ActiveFrames object representing the current frame range.
		 */
		ActiveFrames GetFrames() const { return frames; }


		ActiveFrames& GetFrames() { return frames; }

		/**
		 * @brief Set the animator to a new state, if it has been registered.
		 * @param state The state identifier to switch to.
		 * @throws std::runtime_error if the state was not registered with AddState().
		 */
		void SetState(unsigned int state);
		/**
		 * @brief Add a new animation state.
		 * @param state The state identifier.
		 * @param frames The ActiveFrames definition for the state.
		 *
		 * If the state already exists, no action is taken (prints a warning).
		 */
		void AddState(int state, ActiveFrames const& framerange, const std::string& statename = "");

		/**
		 * @brief Get all registered animation states.
		 * @return An unordered_map of state IDs mapped to their corresponding ActiveFrames.
		 */
		std::unordered_map<unsigned int, ActiveFrames>& GetStateFrames() { return stateframe; }

		void EditFrames(int row, int colstart, int colend, float duration, bool loop);

		bool GetLoop() const { return frames.willLoop; }

		void SetLoop(bool loop) { frames.willLoop = loop; }


		/**
		 * @brief Get the current active animation state.
		 * @return The integer identifier of the current state.
		 */
		int GetCurrentState() const { return m_current_state; }

		void Update(float) override;

		RenderType GetType() const override { return RT_SPRITESHEET; }

		std::unique_ptr<IRenderData> Clone() const override {
			return std::make_unique<SpriteSheet>(*this);
		}

		std::string Serialize() const override;

		static SpriteSheet DeSerialize(Entity eID, const std::string& data);

		Editor::ComponentData EditorData();

		int GetCurrentCol() const { return m_current_col; }
		float GetAnimTimer() const { return anim_timer; }
		void SetCurrentCol(int col) { m_current_col = col; }
		void SetAnimTimer(float t) { anim_timer = t; }
	};

	};

