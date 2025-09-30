#include "Animator.h"
#include <stdio.h>
#include <stdexcept>
#include <iostream>

namespace Animator {

	AnimatorComponent::AnimatorComponent() :
		m_total_rows{}, m_total_cols{}, m_current_row{}, m_current_col{},
		u0{}, v0{}, u1{}, v1{}, anim_duration{}, anim_timer{}, frames{} 
	{ UpdateUV(); }

	AnimatorComponent::AnimatorComponent(int total_rows, int total_cols, ActiveFrames const& framerange, float duration) :
		m_total_rows{ total_rows }, m_total_cols{ total_cols }, m_current_row{}, m_current_col{}, 
		frames { framerange }, anim_timer{}, anim_duration{ duration }
	{
		range = framerange.col_end - framerange.col_start;
		m_uv_width = 1.f / m_total_cols;
		m_uv_height = 1.f / m_total_rows;
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
		if (anim_timer >= anim_duration)
		{
			anim_timer = anim_timer - anim_duration;
		}

		// Assign current row and col
		m_current_row = frames.row;
		m_current_col = frames.col_start + static_cast<int>((anim_timer / anim_duration) * range);
		if (m_current_col > frames.col_end)
		{
			m_current_col = frames.col_end;
		}
		UpdateUV();
		std::cout << "u0 " << u0 << " v0 " << v0 << " u1 " << u1 << " v1 " << v1 << '\n';
	}

}