/* ========================================================
 * @file	Animator.cpp
 * @author	Tan Jun Quan | t.junquan@digipen.edu
 * @date	01 OCT 2025
 *
 * @brief	Animator component for ECS
 *
 * Implementation of the classes and functions declared 
 * in Animator.h
 * These functions facilitate the updating and editing of
 * spritesheet management.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "Animator.h"
#include "Sprite.h"
#include <stdio.h>
#include <stdexcept>
#include <iostream>

namespace Animator {

	bool ActiveFrames::operator==(const ActiveFrames& rhs) const
	{ return (this->row == rhs.row && this->col_start == rhs.col_start && this->col_end == rhs.col_end && this->duration == rhs.duration); }

	AnimatorComponent::AnimatorComponent() :
		m_total_rows{}, m_total_cols{}, m_current_row{}, m_current_col{},
		u0{}, v0{}, u1{}, v1{}, anim_duration{}, anim_timer{}, frames{} 
	{ UpdateUV(); }

	AnimatorComponent::AnimatorComponent(int total_rows, int total_cols, int imageWidthPixels, int imageHeightPixels, ActiveFrames const& framerange, float duration) :
		m_total_rows{ total_rows }, m_total_cols{ total_cols }, m_current_row{}, m_current_col{}, 
		frames { framerange }, anim_timer{}, anim_duration{ duration }
	{
		range = framerange.col_end - framerange.col_start;
		m_uv_width = static_cast<float>(imageWidthPixels) / static_cast<float>(m_total_cols);
		m_uv_height = static_cast<float>(imageHeightPixels) / static_cast<float>(m_total_rows);
		m_current_row = frames.row;
		m_current_col = frames.col_start;
		UpdateUV();
	}

	void AnimatorComponent::UpdateUV()
	{
		u0 = m_current_col * m_uv_width;
		v0 = m_current_row * m_uv_height;
		u1 = u0 + m_uv_width;
		v1 = v0 + m_uv_height;
	}

	void AnimatorComponent::SetFrames(ActiveFrames const& framerange, float duration)
	{
		anim_duration = duration;
		frames = framerange;
		if (duration == 0)
		{
			anim_timer = 0;
			m_current_row = frames.row;
			m_current_col = frames.col_start;
			if (frames.col_end - frames.col_start != 0)
			{ throw std::runtime_error("sumting wong, u call 0 duration but only 1 frame? check ur code or call jq\n"); }
			return;
		}

		// anim_timer = 0;		// okay i cant set this to 0, needs to be able to keep looping for looped animations, but have functionality to call a reset
		
		range = frames.col_end - frames.col_start;
		m_current_row = frames.row;
		m_current_col = frames.col_start;
		UpdateUV();
	}

	void AnimatorComponent::ResetTimer()
	{ anim_timer = 0; }

	void Loop_Timer(float& timer, const float& limit)
	{
		while (timer > limit)
		{ timer -= limit; }
	}


	void AnimatorComponent::UpdateFrame(float dt)
	{

		// If anim_duration == 0, use a single frame for this state
		if (anim_duration == 0)
		{
			m_current_row = frames.row;
			m_current_col = frames.col_start;
			return;
		}

		// Update anim timer and wrap it if it exceeds the duration
		anim_timer += dt;


		// Reset timer
		Loop_Timer(anim_timer, anim_duration);

		// Assign current row and col
		m_current_row = frames.row;
		m_current_col = frames.col_start + static_cast<int>((anim_timer / anim_duration) * (range+1));
		if (m_current_col > frames.col_end)
		{
			m_current_col = frames.col_end;
		}

		UpdateUV();
		// std::cout << "u0 " << u0 << " v0 " << v0 << " u1 " << u1 << " v1 " << v1 << '\n';
	}

	void AnimatorComponent::SetState(int state)
	{
		auto it = stateframe.find(state);
		if (it == stateframe.end())
		{
			std::cout << "U NEVER SET THIS STATE " << state << " BEFORE!!! WHAT U DOING!! I COMPLAIN TO PRASSANA!!\n";
			throw std::runtime_error("NEVER DECLARE STATE THEN GO SET STATE FOR ANIMATOR BOBO\n");
		}

		// HAS frames of this key

		// Change to new frames!
		if (frames != stateframe[state])
		{
			frames = stateframe[state];
			anim_duration = frames.duration;
			m_current_row = frames.row;
			m_current_col = frames.col_start;

			UpdateUV();
			ResetTimer();
		}
		 m_current_state = state;
	}

	void AnimatorComponent::AddState(int state, ActiveFrames const& frames)
	{
		auto it = stateframe.find(state);
		if (it != stateframe.end())
		{
			std::cout << "THIS STATE ALREADY SET!!! but i dont runtime error u bc i feeling nice\n";
			return;
		}

		stateframe[state] = frames;
	}
}