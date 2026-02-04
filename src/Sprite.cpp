/* ========================================================
 * @file	Sprite.cpp
 * @author	Tan Jun Quan | t.junquan@digipen.edu
 * @date	26 OCT 2025
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

	SpriteComponent::SpriteComponent() : m_image(), m_oldtexID{}, m_scale { 0.f, 0.f }, m_rot{ 0.f }, tint{ 0.0f, 0.0f, 0.0f } {}

	SpriteComponent::SpriteComponent(std::string const& path, Vec2 scale_offset) : m_scale{ scale_offset }, m_rot{ 0.f } {
		m_image = CreateImage(path);
		m_oldtexID = m_image.textureID;
	}

	SpriteComponent::SpriteComponent(Image const& image, float const& width, float const& height) :
		m_image(image),
		m_oldtexID(m_image.textureID),
		m_scale(width, height),
		m_rot{ 0.f }
	{}

	SpriteComponent::SpriteComponent(Image const& image, Vec2 const& dimensions) :
		m_image(image),
		m_oldtexID(m_image.textureID),
		m_scale(dimensions),
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

	const Image& SpriteComponent::Get_Image() const
	{
		return m_image;
	}

	const Vec2& SpriteComponent::Get_Scale() const
	{
		return m_scale;
	}
	
	Vec2 SpriteComponent::Get_ImageSize() const
	{
		return { static_cast<float>(m_image.widthPixels), static_cast<float>(m_image.heightPixels) };
	}

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

	void SpriteComponent::Update_ImageSize() {
		if (m_oldtexID == m_image.textureID) { return; }

		glBindTexture(GL_TEXTURE_2D, m_image.textureID);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &m_image.widthPixels);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &m_image.heightPixels);
		glBindTexture(GL_TEXTURE_2D, 0);
		m_oldtexID = m_image.textureID;
		
	}

	void SpriteComponent::Update(float) {
		Update_ImageSize();
	}

	std::string SpriteComponent::Serialize() const {

		json j = {
			{"texturePath", Get_TexturePath()},
			{"scaleX", m_scale.x},
			{"scaleY", m_scale.y},
			{"rot", m_rot},
			{"flipX", flipX},
			{"flipY", flipY}

		};

		return j.dump();
	}

	SpriteComponent SpriteComponent::DeSerialize(Entity eID, const std::string& data) {
		(void)eID;
		if (data.empty()) {
			return SpriteComponent();
		}
		json j = json::parse(data);

		SpriteComponent temp(j["texturePath"], { j["scaleX"], j["scaleY"] });
		if (j.contains("flipX")) { temp.SetFlipX(j["flipX"]); }
		if (j.contains("flipY")) { temp.SetFlipY(j["flipY"]); }

		return temp;
	}

	Editor::ComponentData SpriteComponent::EditorData() {
		Editor::ComponentData ret("Sprite");

		Editor::Widget_Dropdown dropdown{};
		dropdown.ref = reinterpret_cast<unsigned int*>(&m_image.textureID);
		// dropdown.enumMap = Render::GetInstance().Get_TextureCache();
		for (auto const& [optname, optval] : Render::GetInstance().Get_TextureCache())
		{
			dropdown.Add_Option(optname, optval);
		}
		ret.Add_Data("TextureIDtest", dropdown);
		ret.Add_Data("TextureID", Editor::Widget_String{});

		ret.Add_Data("ScaleX", Editor::Widget_Float(&m_scale.x));
		ret.Add_Data("ScaleY", Editor::Widget_Float(&m_scale.y));

		ret.Add_Data("Rotation", Editor::Widget_Float{ &m_rot, true, -360.f, 360.f });

		ret.Add_Data("Flip X", Editor::Widget_Checkmark{ &flipX,false });
		ret.Add_Data("Flip Y", Editor::Widget_Checkmark{ &flipY,false });

		return ret;
	}



	// SPRITESHEET

	SpriteSheet::SpriteSheet() : 
		m_sprite{},
		m_total_rows{1}, m_total_cols{1}, m_current_row{}, m_current_col{},
		m_uv_height{}, m_uv_width{}, anim_duration{}, anim_timer{},
		frames{}, range{}, u0{}, v0{}, u1{}, v1{}
	{}

	bool IsStateCorrect(int row, int col, int state, const std::unordered_map<unsigned int, ActiveFrames>& statemap)
	{
		if (row != statemap.find(state)->second.row) { return false; }
		if (col < statemap.find(state)->second.col_start || col > statemap.find(state)->second.col_end) { return false; }
		return true;
	}

	void SpriteSheet::UpdateUV()
	{
		if (frames != stateframe[m_current_state])
		{
			SetFrames(stateframe[m_current_state]);
		}
		if (m_total_cols == 0) { m_total_cols = 1; }
		if (m_total_rows == 0) { m_total_rows = 1; }
		m_uv_width = static_cast<float>(Sprite().Get_Image().widthPixels) / static_cast<float>(m_total_cols);
		m_uv_height = static_cast<float>(Sprite().Get_Image().heightPixels) / static_cast<float>(m_total_rows);

		int fliprow = (m_total_rows - 1) - m_current_row;

		u0 = m_current_col * m_uv_width;
		v0 = fliprow * m_uv_height;
		u1 = u0 + m_uv_width;
		v1 = v0 + m_uv_height;
	}

	void SpriteSheet::UpdateUV(int imageWidthPixels, int imageHeightPixels)
	{
		if (!IsStateCorrect(m_current_row, m_current_col, m_current_state, stateframe))
		{
			SetFrames(stateframe[m_current_state]);
		}
		m_uv_width = static_cast<float>(imageWidthPixels) / static_cast<float>(m_total_cols);
		m_uv_height = static_cast<float>(imageHeightPixels) / static_cast<float>(m_total_rows);
		m_current_row = frames.row;
		m_current_col = frames.col_start;
		int fliprow = (m_total_rows - 1) - m_current_row;

		u0 = m_current_col * m_uv_width;
		v0 = fliprow * m_uv_height;
		u1 = u0 + m_uv_width;
		v1 = v0 + m_uv_height;
	}

	void SpriteSheet::SetFrames(ActiveFrames const& framerange)
	{
		anim_duration = framerange.duration;
		frames = framerange;
		if (anim_duration == 0)
		{
			anim_timer = 0;
			m_current_row = frames.row;
			m_current_col = frames.col_start;
			if (frames.col_end - frames.col_start <= 0)
			{
				// something wrong here
				// throw std::runtime_error("sumting wong, u call 0 duration but only 1 frame? check ur code or call jq\n");
				return;
			}
			return;
		}

		// anim_timer = 0;		// okay i cant set this to 0, needs to be able to keep looping for looped animations, but have functionality to call a reset

		range = frames.col_end - frames.col_start;
		m_current_row = frames.row;
		m_current_col = frames.col_start;
		UpdateUV();
	}

	void SpriteSheet::ResetTimer()
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

	void SpriteSheet::UpdateFrame(float dt)
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
		if (frames.willLoop)
		Loop_Timer(anim_timer, anim_duration);

		// Assign current row and col
		range = frames.col_end - frames.col_start;
		m_current_row = frames.row;
		m_current_col = frames.col_start + static_cast<int>((anim_timer / anim_duration) * (range + 1));
		if (m_current_col > frames.col_end)
		{
			m_current_col = frames.col_end;
		}

		UpdateUV();
		// std::cout << "u0 " << u0 << " v0 " << v0 << " u1 " << u1 << " v1 " << v1 << '\n';
	}

	void SpriteSheet::SetState(unsigned int state)
	{
		auto it = stateframe.find(state);
		if (it == stateframe.end())
		{
			std::cout << "U NEVER SET THIS STATE " << state << " BEFORE!!! WHAT U DOING!!!\n";
			SetState(0);
			return;
			// throw std::runtime_error("NEVER DECLARE STATE THEN GO SET STATE FOR SPRITESHEET\n");
		}

		// HAS frames of this key

		// Change to new frames!
		if (frames != stateframe[state])
		{
			frames = stateframe[state];
			anim_duration = frames.duration;
			m_current_row = frames.row;
			m_current_col = frames.col_start;
			frames.willLoop = stateframe[state].willLoop;

			UpdateUV();
			ResetTimer();
		}
		m_current_state = state;
	}

	void SpriteSheet::AddState(int state, ActiveFrames const& framerange, std::string const& statename)
	{
		auto it = stateframe.find(state);
		if (it != stateframe.end())
		{
			std::cout << "THIS STATE ALREADY SET!!! but i dont runtime error u bc i feeling nice\n";
			return;
		}
		std::string name = statename;
		if (name == "") { name = std::to_string(state); }

		stateNames[statename] = state;
		stateframe[state] = framerange;
	}

	void SpriteSheet::EditFrames(int row, int colstart, int colend, float duration, bool loop) {
		stateframe[m_current_state].row = row;
		stateframe[m_current_state].col_start = colstart;
		stateframe[m_current_state].col_end = colend;
		stateframe[m_current_state].duration = duration;
		stateframe[m_current_state].willLoop = loop;
	}

	void SpriteSheet::Update(float dt) {
		m_sprite.Update_ImageSize();

		UpdateFrame(dt);
	}

	std::string SpriteSheet::Serialize() const {
		json statesJson = json::object();

		for (const auto& [state, stateFrames] : stateframe)
		{
			statesJson[std::to_string(state)] = { 
				{"row", stateFrames.row}, 
				{"col_start", stateFrames.col_start}, 
				{"col_end", stateFrames.col_end}, 
				{"duration", stateFrames.duration},
				{"willLoop", stateFrames.willLoop}
			};
		}

		json j = {
			{"sprite", m_sprite.Serialize()},

			{"totalRows", m_total_rows},
			{"totalCols", m_total_cols},
			{"animDuration", anim_duration},
			{"frameRow", GetFrames().row},
			{"frameColStart", GetFrames().col_start},
			{"frameColEnd", GetFrames().col_end},
			{"currentState", GetCurrentState()},
			{"states", statesJson},
			{"loop", frames.willLoop}


		};

		j["sprite"] = m_sprite.Serialize();

		return j.dump();

	}

	SpriteSheet SpriteSheet::DeSerialize(Entity eID, const std::string& data) {
		(void)eID;
		if (data.empty()) {
			return SpriteSheet();
		}
		json j = json::parse(data);

		SpriteSheet ret{};

		if (j.contains("sprite")) {
			ret.m_sprite = SpriteComponent::DeSerialize(eID,j["sprite"]);
		}

		ActiveFrames frames;
		frames.row = j.value("frameRow", 0);
		frames.col_start = j.value("frameColStart", 0);
		frames.col_end = j.value("frameColEnd", 0);
		frames.duration = j.value("frameDuration", 1.0f);
		frames.willLoop = j.value("loop", false);

		// Get basic values
		ret.m_total_rows = j.value("totalRows", 1);
		ret.m_total_cols = j.value("totalCols", 1);
		ret.anim_duration = j.value("animDuration", 0.0f);
		ret.anim_timer = 0;

		int imageWidth	= ret.m_sprite.Get_Image().widthPixels;
		int imageHeight = ret.m_sprite.Get_Image().heightPixels;


		if (j.contains("states")) {
			for (auto& [stateKey, stateData] : j["states"].items()) {
				int stateID = std::stoi(stateKey);
				ActiveFrames stateFrames;
				stateFrames.row = stateData.value("row", 0);
				stateFrames.col_start = stateData.value("col_start", 0);
				stateFrames.col_end = stateData.value("col_end", 0);
				stateFrames.duration = stateData.value("duration", 1.0f);
				stateFrames.willLoop = stateData.value("willLoop", false);


				ret.AddState(stateID, stateFrames);
				ret.SetState(stateID);
			}
		}
		//if (j.contains("currentState"))
		//{
		//	auto tempy = j["currentState"];
		//	temp.SetState(j["currentState"]);
		//}

		// check for null
		if (j.contains("currentState") && !j["currentState"].is_null() && j["currentState"] != -1) {
			ret.SetState(j["currentState"]);
		}

		if (j.contains("loop")) {
			ret.SetLoop(j["loop"]);
		}

		ret.UpdateUV(imageWidth, imageHeight);

		return ret;
	}

	Editor::ComponentData SpriteSheet::EditorData() {
		Editor::ComponentData ret("SpriteSheet");

		//ret.Add_Data("Total Rows", Editor::Widget_Int(&m_total_rows));
		//ret.Add_Data("Total Cols", Editor::Widget_Int(&m_total_cols));

		//ret.Add_Data("Total Rows", Editor::Widget_Int{ reinterpret_cast<int*>(&m_total_rows) });
		//ret.Add_Data("Total Cols", Editor::Widget_Int{ reinterpret_cast<int*>(&m_total_cols) });

		Editor::Widget_Animator animwidget(&stateframe, &m_total_rows, &m_total_cols, &m_current_state);
		/*for (auto& [name, state] : stateNames)
		{
			animwidget.current_state.Add_Option(name, state);
		}*/

		ret.Add_Data("Animator values", animwidget);
		// ret.Add_Data("Animator States", animwidget);

		return ret;
	}
}