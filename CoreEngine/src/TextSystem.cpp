/* ========================================================
 * @file	    TextSystem.cpp
 * @author	    Manfred Sim | manfred.sim@digipen.edu 100%
 * @date	    30 OCT 2025
 *
 * @brief		Implementation of the Text System for handling
				dialogue boxes for the GUI system.
 *
 * The Text System is responsible for rendering text and
 * handling typing behavior of dialogue boxes used by the
 * UI editor. It also contains implementation of the TextComponent
 * that will be used for defining dialogue boxes and helper
 * functions that are used to simulate the typing of dialogue text.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "FilePath.h"
#include "TextSystem.h"

namespace TextSystem
{

/**
 * @brief	Default constructor for TextComponent.
 *			Initializes all member variables to their default values.
 */

	TextComponent::TextComponent() : lines{ "" }, currentLineIndex{ 0 }, textSpeed{ 0.f }, speakerID{ 0 }, fontPath{}, fontSize{ 0 }, isTyping{}, color{} {}

/**
 * @brief Parameterized constructor for TextComponent.
 *
 * @param lines Vector of dialogue lines.
 * @param currentLineIndex Index of the current line.
 * @param textSpeed Speed at which text is revealed.
 * @param wrapWidth Maximum width of rendering text before needing a newline.
 * @param customSpeaker Whether the speaker name can be changed or not.
 * @param speakerName Name of the speaker.
 * @param speakerID ID of the speaker.
 * @param fontPath Path to the font file.
 * @param fontSize Size of the font.
 * @param isTyping Whether the text is currently being typed out.
 * @param color Color of the text.
 */

	TextComponent::TextComponent(std::vector<std::string> lines, int currentLineIndex, float textSpeed, float wrapWidth, std::string, int speakerID, std::string fontPath, float fontSize, bool Typing, Vec3 color)
		: lines{ lines }, currentLineIndex{ currentLineIndex }, textSpeed{ textSpeed }, wrapWidth{ wrapWidth }, speakerID { speakerID }, fontPath{ fontPath }, fontSize{ fontSize }, isTyping{ Typing }, color{ color } 
	{
	}

	// GETTERS

/**
 * @brief Gets the dialogue lines.
 * @return Reference to the vector of dialogue lines.
 */

	const std::vector<std::string>& TextComponent::GetLines() const noexcept { return lines; }

/**
 * @brief Gets the current line index.
 * @return Index of the current line.
 */

	int	TextComponent::GetCurrentLineIndex() const noexcept { return currentLineIndex; }

/**
 * @brief Gets the text speed.
 * @return Speed at which text is revealed.
 */

	float TextComponent::GetTextSpeed() const noexcept { return textSpeed; }

/**
 * @brief Gets the wrap width.
 * @return Maximum width of rendering text before needing a newline.
 */

	float TextComponent::GetWrapWidth() const noexcept { return wrapWidth; }

/**
 * @brief Gets the speaker ID.
 * @return ID of the speaker.
 */

	int TextComponent::GetSpeakerID() const noexcept { return speakerID; }

/**
 * @brief Gets the font path.
 * @return Reference to the font file path.
 */

	const std::string& TextComponent::GetFontPath() const noexcept { return fontPath; }

/**
 * @brief Gets the font size.
 * @return Size of the font.
 */

	float TextComponent::GetFontSize() const noexcept { return fontSize; }

/**
 * @brief Checks if the text is currently being typed out.
 * @return True if typing, false otherwise.
 */

	bool TextComponent::CheckIfTyping() const noexcept { return isTyping; }

/**
 * @brief Gets the color of the text.
 * @return Reference to the color vector.
 */

	const Vec3& TextComponent::GetColor() const noexcept { return color; }

/**
 * @brief Gets the time elapsed of the text typing
 * @return Reference to time elapsed.
 */

	float TextComponent::GetTimeElapsed() const noexcept { return timeElapsed; }

/**
 * @brief Gets the current line of text.
 * @param index Index of the line to retrieve.
 * @return Reference to the string at the specified index.
 */

	const std::string& TextComponent::GetCurrentLine(int index) const noexcept { return lines[index]; }

/**
 * @brief Gets the current number of visible characters.
 * @return Reference to the number of visible characters.
 */

	int TextComponent::GetVisibleChars() const noexcept { return visibleChars; }

	// SETTERS

/**
 * @brief Sets the dialogue lines.
 * @param script Vector of dialogue lines.
 */

	void TextComponent::SetLines(const std::vector<std::string>& script) { lines = script; }

/**
 * @brief Sets the current line index.
 * @param index Index to set as current.
 */

	void TextComponent::SetCurrentLineIndex(int index) { currentLineIndex = index; }

/**
 * @brief Sets the text speed.
 * @param speed Speed at which text is revealed.
 */

	void TextComponent::SetTextSpeed(float speed) { textSpeed = speed; }

/**
 * @brief Sets the wrap width.
 * @param width Maximum width of rendering text before needing a newline.
 */

	void TextComponent::SetWrapWidth(float width) { wrapWidth = width; }

/**
 * @brief Sets the speaker ID.
 * @param id ID of the speaker.
 */

	void TextComponent::SetSpeakerID(int id) { speakerID = id; }

/**
 * @brief Sets the font path.
 * @param path Path to the font file.
 */

	void TextComponent::SetFontPath(const std::string& path) { fontPath = path; }

/**
 * @brief Sets the font size.
 * @param size Size of the font.
 */

	void TextComponent::SetFontSize(float size) { fontSize = size; }

/**
 * @brief Sets whether the text is being typed out.
 * @param isType True if typing, false otherwise.
 */

	void TextComponent::SetIsTyping(bool isType) { isTyping = isType; }

/**
 * @brief Sets the color of the text.
 * @param colorVec Color vector to set.
 */

	void TextComponent::SetColor(Vec3 const& colorVec) { color = colorVec; }

/**
 * @brief Sets the visible characters of the text.
 * @param visibleChars Number of visible characters (from left to right of each line).
 */

	void TextComponent::SetVisibleChars(int charCount) { visibleChars = charCount; }

/**
 * @brief Sets the time elapsed for the text typing.
 * @param charCount Time elapsed while typing the text.
 */

	void TextComponent::SetTimeElapsed(float time) { timeElapsed = time; }

/**
 * @brief	Updates the typing animation for the text. 
 *			Advances the number of visible characters based on elapsed time and speed.
 *
 * @param txt Reference to the TextComponent to update.
 * @param dt Delta time since last update.
 * @param speedMul Speed multiplier for typing.
 */

	void UpdateTyping(TextComponent& txt, float dt, float speedMul)
	{
		if (!txt.isTyping) return;
		dt = std::clamp(dt, 0.f, 1.f / 30.f);

		const std::string& fullLine = txt.GetCurrentLine(txt.GetCurrentLineIndex());
		if (fullLine.empty()) { txt.isTyping = false; txt.visibleChars = 0; txt.timeElapsed = 0.f; return; }

		const float cps = std::max(0.f, txt.textSpeed) * std::max(0.f, speedMul);

		txt.timeElapsed += dt;

		const int target = (int)(txt.timeElapsed * cps);
		txt.visibleChars = std::min((int)fullLine.size(), std::max(txt.visibleChars, target));

		if (txt.visibleChars >= (int)fullLine.size())
		{
			txt.visibleChars = (int)fullLine.size();
			txt.isTyping = false;
		}
	}

/**
 * @brief	Gets the currently visible portion of the text.
 *			Returns a substring of the current line based on the number of visible characters.
 * @param txt Reference to the TextComponent.
 * @return String containing the visible text.
 */

	std::string GetVisibleText(const TextComponent& txt)
	{
		const std::string& fullLine = txt.GetCurrentLine(txt.GetCurrentLineIndex());
		const int n = std::clamp(txt.visibleChars, 0, (int)fullLine.size());
		return fullLine.substr(0, n);
	}

/**
 * @brief	Loads dialogue data from a JSON file into a TextComponent.
 *			Reads a JSON file, parses dialogue lines and settings, and applies them to the TextComponent.
 * @param filepath Path to the JSON file.
 * @param textComp Reference to the TextComponent to populate.
 * @return True if loading was successful, false otherwise.
 */

	bool LoadDialogueFromJson(const std::string& filepath, TextComponent& textComp)
	{
		// Default values
		std::vector<std::string> lines;
		float textSpeed = 60.f;
		Vec3 color = { 1.f, 1.f, 1.f };
		std::string fontPath = (FilePath::ASSETS_FOLDER + "fonts/Marker.ttf");
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