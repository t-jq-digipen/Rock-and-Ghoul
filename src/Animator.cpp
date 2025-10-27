/* ========================================================
 * @file	Animator.cpp
 * @author	Tan Jun Quan | t.junquan@digipen.edu
 * @date	26 OCT 2025
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
#include "Registry.h"

namespace Animator {

	bool ActiveFrames::operator==(const ActiveFrames& rhs) const
	{
		return (this->row == rhs.row && this->col_start == rhs.col_start && this->col_end == rhs.col_end && this->duration == rhs.duration);
	}

	AnimatorComponent::AnimatorComponent() :
		m_total_rows{}, m_total_cols{}, m_current_row{}, m_current_col{},
		u0{}, v0{}, u1{}, v1{}, anim_duration{}, anim_timer{}, frames{}
	{
		UpdateUV();
	}

	AnimatorComponent::AnimatorComponent(int total_rows, int total_cols, int imageWidthPixels, int imageHeightPixels, ActiveFrames const& framerange, float duration) :
		m_total_rows{ total_rows }, m_total_cols{ total_cols }, m_current_row{}, m_current_col{},
		frames{ framerange }, anim_timer{}, anim_duration{ duration }
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
			{
				throw std::runtime_error("sumting wong, u call 0 duration but only 1 frame? check ur code or call jq\n");
			}
			return;
		}

		// anim_timer = 0;		// okay i cant set this to 0, needs to be able to keep looping for looped animations, but have functionality to call a reset

		range = frames.col_end - frames.col_start;
		m_current_row = frames.row;
		m_current_col = frames.col_start;
		UpdateUV();
	}

	void AnimatorComponent::ResetTimer()
	{
		anim_timer = 0;
	}

	void Loop_Timer(float& timer, const float& limit)
	{
		while (timer > limit)
		{
			timer -= limit;
		}
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
		m_current_col = frames.col_start + static_cast<int>((anim_timer / anim_duration) * (range + 1));
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

	void AnimatorComponent::AddState(int state, ActiveFrames const& framerange)
	{
		auto it = stateframe.find(state);
		if (it != stateframe.end())
		{
			std::cout << "THIS STATE ALREADY SET!!! but i dont runtime error u bc i feeling nice\n";
			return;
		}

		stateframe[state] = framerange;
	}

	std::string AnimatorComponent::Serialize() const {
		json statesJson = json::object();

		for (const auto& [state, stateFrames] : stateframe)
		{
			statesJson[std::to_string(state)] = { {"row", stateFrames.row}, {"col_start", stateFrames.col_start}, {"col_end", stateFrames.col_end}, {"duration", stateFrames.duration} };
		}

		json j = {
		{"totalRows", m_total_rows},
		{"totalCols", m_total_cols},
		{"animDuration", anim_duration},
		{"frameRow", GetFrames().row},
		{"frameColStart", GetFrames().col_start},
		{"frameColEnd", GetFrames().col_end},
		{"currentState", GetCurrentState()},
		{"states", statesJson}
		};
		return j.dump();
	}

	AnimatorComponent AnimatorComponent::DeSerialize(Entity eID, const std::string& data) {
		json j = json::parse(data);

		ActiveFrames frames;
		frames.row = j.value("frameRow", 0);
		frames.col_start = j.value("frameColStart", 0);
		frames.col_end = j.value("frameColEnd", 0);
		frames.duration = j.value("frameDuration", 1.0f);

		// Get basic values
		int totalRows = j.value("totalRows", 1);
		int totalCols = j.value("totalCols", 1);
		float animDuration = j.value("animDuration", 0.0f);

		int imageWidth = Registry::Get().Get_Component<Sprite::SpriteComponent>(eID).Get_Image().widthPixels;
		int imageHeight = Registry::Get().Get_Component<Sprite::SpriteComponent>(eID).Get_Image().heightPixels;

		//int imageWidth = j.value("imageWidth", 0);
		//int imageHeight = j.value("imageHeight", 0);

		//if (imageWidth == 0 || imageHeight == 0) {
		//	std::cout << "NEE NAW WARNING: USING DEFAULTS FOR ANIMATOR!!!\n";

		//	// 32x32 pixels per frame as default val 
		//	if (imageWidth == 0) {
		//		imageWidth = totalCols * 32;
		//	}
		//	if (imageHeight == 0) {
		//		imageHeight = totalRows * 32;
		//	}
		//}

		AnimatorComponent temp(totalRows, totalCols, imageWidth, imageHeight, frames, animDuration);

		if (j.contains("states")) {
			for (auto& [stateKey, stateData] : j["states"].items()) {
				int stateID = std::stoi(stateKey);
				ActiveFrames stateFrames;
				stateFrames.row = stateData.value("row", 0);
				stateFrames.col_start = stateData.value("col_start", 0);
				stateFrames.col_end = stateData.value("col_end", 0);
				stateFrames.duration = stateData.value("duration", 1.0f);

				temp.AddState(stateID, stateFrames);
				temp.SetState(stateID);
			}
		}
		//if (j.contains("currentState"))
		//{
		//	auto tempy = j["currentState"];
		//	temp.SetState(j["currentState"]);
		//}

		// check for null
		if (j.contains("currentState") && !j["currentState"].is_null() && j["currentState"] != -1) {
			temp.SetState(j["currentState"]);
		}

		return temp;
	}

	Editor::ComponentData AnimatorComponent::EditorData() {
		Editor::ComponentData ret("Animator");

		ret.Add_Data("Total Rows", Editor::Widget_Int(&m_total_rows));
		ret.Add_Data("Total Cols", Editor::Widget_Int(&m_total_cols));


		return ret;
	}
}