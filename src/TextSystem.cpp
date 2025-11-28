/* ========================================================
 * @file	    TextSystem.cpp
 * @author	    Manfred Sim | manfred.sim@digipen.edu
 * @date	    30 OCT 2025
 *
 * @brief		Implementation of the Text System for the GUI editor.
 *
 * The Text System is responsible for rendering text and
 * handling typing behavior of dialogue boxes used by the
 * GUI editor. It also contains implementation of the TextComponent
 * that will be used for defining dialogue boxes and helper
 * functions that are used to simulate the typing of dialogue text.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "TextSystem.h"

namespace TextSystem
{
	TextComponent::TextComponent() : lines{ "" }, currentLineIndex{ 0 }, textSpeed{ 0.f }, speakerID{ 0 }, fontPath{}, fontSize{ 0 }, isTyping{}, color{} {}
	TextComponent::TextComponent(std::vector<std::string> lines, int currentLineIndex, float textSpeed, int speakerID, std::string fontPath, float fontSize, bool Typing, Vec3 color)
		: lines{ lines }, currentLineIndex{ currentLineIndex }, textSpeed{ textSpeed }, speakerID{ speakerID }, fontPath{ fontPath }, fontSize{ fontSize }, isTyping{ Typing }, color {
		color
	} {
	}

	// GETTERS
	const std::vector<std::string>& TextComponent::GetLines() const noexcept { return lines; }
	int	TextComponent::GetCurrentLineIndex() const noexcept { return currentLineIndex; }
	float TextComponent::GetTextSpeed() const noexcept { return textSpeed; }
	int TextComponent::GetSpeakerID() const noexcept { return speakerID; }
	const std::string& TextComponent::GetFontPath() const noexcept { return fontPath; }
	float TextComponent::GetFontSize() const noexcept { return fontSize; }
	bool TextComponent::CheckIfTyping() const noexcept { return isTyping; }
	const Vec3& TextComponent::GetColor() const noexcept { return color; }
	const std::string& TextComponent::GetCurrentLine(int index) const noexcept { return lines[index]; }

	// SETTERS
	void TextComponent::SetLines(const std::vector<std::string>& script) { lines = script; }
	void TextComponent::SetCurrentLineIndex(int index) { currentLineIndex = index; }
	void TextComponent::SetTextSpeed(float speed) { textSpeed = speed; }
	void TextComponent::SetSpeakerID(int id) { speakerID = id; }
	void TextComponent::SetFontPath(const std::string& path) { fontPath = path; }
	void TextComponent::SetFontSize(float size) { fontSize = size; }
	void TextComponent::SetIsTyping(bool isType) { isTyping = isType; }
	void TextComponent::SetColor(Vec3 const& colorVec) { color = colorVec; }

	// Toggle text behavior (if isTyping = true, text gets rendered from left to right)
	void UpdateTyping(TextComponent& txt, float dt, float speedMul)
	{
		if (!txt.isTyping) return;
		txt.timeElapsed = 0.f;
		const std::string& fullLine = txt.GetCurrentLine(txt.GetCurrentLineIndex());
		if (fullLine.empty()) { txt.isTyping = false; txt.visibleChars = 0; return; }

		txt.timeElapsed += std::max(0.f, dt);
		const float cps = std::max(0.f, txt.textSpeed) * std::max(0.f, speedMul);
		const int target = (int)(txt.timeElapsed * cps);

		if (target > txt.visibleChars) txt.visibleChars = target;

		if (txt.visibleChars >= (int)fullLine.size())
		{
			txt.visibleChars = (int)fullLine.size();
			txt.isTyping = false;
		}
	}

	// Render all visible text
	std::string GetVisibleText(const TextComponent& txt)
	{
		const std::string& fullLine = txt.GetCurrentLine(txt.GetCurrentLineIndex());
		const int n = std::clamp(txt.visibleChars, 0, (int)fullLine.size());
		return fullLine.substr(0, n);
	}

	bool LoadDialogueFromJson(const std::string& filepath, TextComponent& textComp)
	{
		// Default values
		std::vector<std::string> lines;
		float textSpeed = 30.f;
		Vec3 color = { 1.f, 1.f, 1.f };
		std::string fontPath = "../Assets/fonts/Marker.ttf";
		float fontSize = 1.f;
		int speakerID = 0;

		// Read file
		const std::string content = FileIO::ReadFile(filepath);
		if (content.empty()) 
		{
			std::cerr << "Failed to load dialogue from: " << filepath << '\n';
			return false;
		}

		json j;
		try 
		{
			j = json::parse(content);
		}
		catch (const std::exception& e) 
		{
			std::cerr << "JSON parse error in " << filepath << ": " << e.what() << "\n";
			return false;
		}

		if (!j.contains("scenes") || !j["scenes"].is_array() || j["scenes"].empty()) 
		{
			std::cerr << "Missing scene in " << filepath << "\n";
			return false;
		}

		const json& scene = j["scenes"][0];

		if (scene.contains("script") && scene["script"].is_array()) 
		{
			for (const auto& entry : scene["script"]) 
			{
				if (entry.contains("line") && entry["line"].is_string()) 
				{
					lines.push_back(entry["line"].get<std::string>());
				}
			}
		}

		if (scene.contains("textSpeed") && scene["textSpeed"].is_number()) 
		{
			textSpeed = scene["textSpeed"].get<float>();
		}
		if (scene.contains("color") && scene["color"].is_array() && scene["color"].size() == 3) 
		{
			color = 
			{
				scene["color"][0].get<float>(),
				scene["color"][1].get<float>(),
				scene["color"][2].get<float>()
			};
		}
		if (scene.contains("fontPath") && scene["fontPath"].is_string()) 
		{
			fontPath = scene["fontPath"].get<std::string>();
		}
		if (scene.contains("fontSize") && scene["fontSize"].is_number()) 
		{
			fontSize = scene["fontSize"].get<float>();
		}

		// Apply to TextComponent
		textComp.SetLines(lines);
		textComp.SetCurrentLineIndex(0);
		textComp.SetTextSpeed(textSpeed);
		textComp.SetSpeakerID(speakerID);
		textComp.SetFontPath(fontPath);
		textComp.SetFontSize(fontSize);
		textComp.SetColor(color);

		// Reset typewriter state
		textComp.SetIsTyping(false);

		return true;
	}
}