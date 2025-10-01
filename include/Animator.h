/* ========================================================
 * @file	Animator.h
 * @author	Tan Jun Quan
 * @date	01 OCT 2025
 *
 * @brief	Animator component for ECS
 *
 * Anim things (BRIEF TO BE DONE)
 *
 *
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once

namespace Animator {

	// ActiveFrames set the rows and cols to be used
	// Rows and cols are called as follows:
	//   C0 C1 C2 C3 
	// R3
	// R2
	// R1 x  x  x
	// R0
	// 
	// e.g	if I would like to call the frames marked X, ActiveFrames would be
	//		initialized as {1, 0, 2}
	struct ActiveFrames {
		int row = 0;
		int col_start = 0;
		int col_end = 0;
	};

	class AnimatorComponent {
	private:
		int m_total_rows;
		int m_total_cols;

		int m_current_row;
		int m_current_col;

		float m_uv_height;
		float m_uv_width;


		float anim_duration;	// Time to cycle through the frames in seconds
		float anim_timer;		// Time elapsed

		ActiveFrames frames;	// Which frames to call
		int range;				// Number of frames we are looping through

	public:
		float u0, v0, u1, v1;	// For sub image

		AnimatorComponent();

		AnimatorComponent(int total_rows, int total_cols, int imageWidthPixels, int imageHeightPixels, ActiveFrames const& framerange, float duration = 0.f);

		void UpdateUV();

		void SetFrames(ActiveFrames const& framerange, float duration);

		void ResetTimer();

		void UpdateFrame(float dt);

		
	};

}