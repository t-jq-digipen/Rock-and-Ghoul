/* ========================================================
 * @file	Animator.h
 * @author	Tan Jun Quan | t.junquan@digipen.edu
 * @date	26 OCT 2025
 *
 * @brief	Animator component for ECS
 *
 * Provides classes and structures for handling 
 * spritesheet animations.
 * 
 * ActiveFrames usage guide:
 * For frames marked with X, initialize with {1,0,2}
 * 
 *   C0 C1 C2 C3
 * R3
 * R2
 * R1 X  X  X
 * R0
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include <unordered_map>
#include "DataDefs.h"
#include <nlohmann/json.hpp>
namespace Animator {

	/**
	* @struct ActiveFrames
	* @brief Represents a set of frames for a given animation and a duration it should take to play it.
	*/
	struct ActiveFrames {
		int row			= 0;	// Row index
		int col_start	= 0;	// First column index
		int col_end		= 0;	// Last column index
		float duration	= 0.f;	// Duration of time in seconds the animation takes to go from first column to last column

		/**
		 * @brief Equality operator for ActiveFrames.
		 * @param rhs The ActiveFrames object to compare with.
		 * @return True if all fields (row, col_start, col_end, duration) match, false otherwise.
		 */
		bool operator==(const ActiveFrames& rhs) const;
	};



	/**
	* @class AnimatorComponent
	* @brief Represents an animator component for game objects.
	*		 This component handles which sub-image the entity should be drawing out.
	*/
	class AnimatorComponent {
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
		std::unordered_map<int, ActiveFrames> stateframe;	// A map with int key for active frames

		// Using int for enumeration compatibility with developers
		int m_current_state = -1;							// Animation state

	public:
		float u0, v0, u1, v1;								// For sub image drawing


		/**
		 * @brief Default ctor. Initializes all members to zero/defaults.
		 */
		AnimatorComponent();
		
		/**
		 * @brief Construct an AnimatorComponent with image grid and frame information.
		 * @param total_rows Total number of rows in the sprite sheet.
		 * @param total_cols Total number of columns in the sprite sheet.
		 * @param imageWidthPixels Width of the sprite sheet in pixels.
		 * @param imageHeightPixels Height of the sprite sheet in pixels.
		 * @param framerange The range of frames (row and start/end columns) for the animation.
		 * @param duration Duration (in seconds) for one animation loop.
		 */
		AnimatorComponent(int total_rows, int total_cols, ActiveFrames const& framerange, float duration = 0.f);

		/**
		 * @brief Update UV coordinates based on the current row and column.
		 */
		void UpdateUV();

		void UpdateUV(int imageWidthPixels, int imageHeightPixels);

		/**
		 * @brief Set the active animation frames and duration.
		 * @param framerange The range of frames (row and column indices).
		 * @param duration Duration (in seconds) of the animation cycle. Zero means a single-frame animation.
		 * @throws std::runtime_error if duration == 0 but multiple frames are specified.
		 */
		void SetFrames(ActiveFrames const& framerange, float duration);

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
		/**
		 * @brief Get the total number of columns in the sprite sheet.
		 * @return The number of columns.
		 */
		int GetTotalCols() const { return m_total_cols; }
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

		/**
		 * @brief Set the animator to a new state, if it has been registered.
		 * @param state The state identifier to switch to.
		 * @throws std::runtime_error if the state was not registered with AddState().
		 */
		void SetState(int state);
		/**
		 * @brief Add a new animation state.
		 * @param state The state identifier.
		 * @param frames The ActiveFrames definition for the state.
		 *
		 * If the state already exists, no action is taken (prints a warning).
		 */
		void AddState(int state, ActiveFrames const& framerange);

		/**
		 * @brief Get all registered animation states.
		 * @return An unordered_map of state IDs mapped to their corresponding ActiveFrames.
		 */
		std::unordered_map<int, ActiveFrames> GetStateFrames() const { return stateframe; }
		/**
		 * @brief Get the current active animation state.
		 * @return The integer identifier of the current state.
		 */
		int GetCurrentState() const { return m_current_state; }

		std::string Serialize() const;

		static AnimatorComponent DeSerialize(Entity eID, const std::string& data);
		
		Editor::ComponentData EditorData();
	};

}