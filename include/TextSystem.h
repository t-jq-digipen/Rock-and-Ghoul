/* ========================================================
 * @file	    TextSystem.h
 * @author	    Manfred Sim | manfred.sim@digipen.edu
 * @date	    30 OCT 2025
 *
 * @brief		Declares the Text System for the GUI editor.
 *
 * The Text System is responsible for rendering text and
 * handling typing behavior of dialogue boxes used by the
 * GUI editor. This file also contains declaration of the 
 * TextComponent that will be used for defining dialogue boxes
 * and helper functions that are used to simulate the
 * typing of dialogue text.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <Remath.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include "FileIO.h"

using json = nlohmann::json;

namespace TextSystem
{
	class TextComponent {
		std::vector<std::string> lines;
		int currentLineIndex{};
		float textSpeed{};
		float wrapWidth{};
		//bool customSpeaker{ false };
		//std::string speakerName;
		int speakerID{};
		std::string fontPath;
		float fontSize{ 0 };
		Vec3 color;

		// while typing (not serialized)
		bool isTyping{ false };
		float timeElapsed{ 0.f };
		int visibleChars{ 0 };

	public:

/**
 * @brief	Default constructor for TextComponent.
 *			Initializes all member variables to their default values.
 */

		TextComponent();

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

		TextComponent(std::vector<std::string> lines, int currentLineIndex,
			float textSpeed, float wrapWidth, bool customSpeaker, std::string speakerName, int speakerID, 
			std::string fontPath, float fontSize, bool isTyping, Vec3 color);

/**
 * @brief	Copy constructor for TextComponent.
 */

		TextComponent(const TextComponent& other) = default;

		// GETTERS

/**
 * @brief Gets the dialogue lines.
 * @return Reference to the vector of dialogue lines.
 */

		const std::vector<std::string>& GetLines() const noexcept;

/**
 * @brief Gets the current line index.
 * @return Index of the current line.
 */

		int	GetCurrentLineIndex() const noexcept;

/**
 * @brief Gets the text speed.
 * @return Speed at which text is revealed.
 */

		float GetTextSpeed() const noexcept;

/**
 * @brief Gets the wrap width.
 * @return Maximum width of rendering text before needing a newline.
 */

		float GetWrapWidth() const noexcept;

/**
 * @brief Gets the speaker ID.
 * @return ID of the speaker.
 */

		int GetSpeakerID() const noexcept;

/**
 * @brief Gets the font path.
 * @return Reference to the font file path.
 */

		const std::string& GetFontPath() const noexcept;

/**
 * @brief Gets the font size.
 * @return Size of the font.
 */

		float GetFontSize() const noexcept;

/**
 * @brief Checks if the text is currently being typed out.
 * @return True if typing, false otherwise.
 */

		bool CheckIfTyping() const noexcept;

/**
 * @brief Gets the color of the text.
 * @return Reference to the color vector.
 */

		const Vec3& GetColor() const noexcept;

/**
 * @brief Gets the time elapsed of the text typing
 * @return Reference to time elapsed.
 */

		float GetTimeElapsed() const noexcept;

/**
 * @brief Gets the current line of text.
 * @param index Index of the line to retrieve.
 * @return Reference to the string at the specified index.
 */

		const std::string& GetCurrentLine(int index) const noexcept;

		// SETTERS

/**
 * @brief Sets the dialogue lines.
 * @param script Vector of dialogue lines.
 */

		void SetLines(const std::vector<std::string>& script);

/**
 * @brief Sets the current line index.
 * @param index Index to set as current.
 */

		void SetCurrentLineIndex(int index);

/**
 * @brief Sets the text speed.
 * @param speed Speed at which text is revealed.
 */

		void SetTextSpeed(float speed);

/**
 * @brief Sets the wrap width.
 * @param width Maximum width of rendering text before needing a newline.
 */

		void SetWrapWidth(float width);

/**
 * @brief Sets the speaker ID.
 * @param id ID of the speaker.
 */

		void SetSpeakerID(int id);

/**
 * @brief Sets the font path.
 * @param path Path to the font file.
 */

		void SetFontPath(const std::string& path);

/**
 * @brief Sets the font size.
 * @param size Size of the font.
 */

		void SetFontSize(float size);

/**
 * @brief Sets whether the text is being typed out.
 * @param isType True if typing, false otherwise.
 */

		void SetIsTyping(bool isType);

/**
 * @brief Sets the color of the text.
 * @param colorVec Color vector to set.
 */

		void SetColor(Vec3 const& colorVec);


/**
 * @brief Sets the visible characters of the text.
 * @param charCount Number of visible characters (from left to right of each line).
 */

		void SetVisibleChars(int charCount);

/**
 * @brief Sets the time elapsed for the text typing.
 * @param charCount Time elapsed while typing the text.
 */

		void SetTimeElapsed(float time);

		// TYPE TEXT CALL FUNCTIONS

/**
 * @brief Starts the typing of dialogue boxes.
 */

		void StartTyping() { isTyping = true; timeElapsed = 0.f; visibleChars = 0; }

/**
 * @brief Stops the typing of dialogue boxes and shows the full current line.
 */

		void StopTyping() { isTyping = false; visibleChars = static_cast<int>(GetCurrentLine(GetCurrentLineIndex()).size()); }

/**
 * @brief	Updates the typing animation for the text. 
 *			Advances the number of visible characters based on elapsed time and speed.
 *
 * @param txt Reference to the TextComponent to update.
 * @param dt Delta time since last update.
 * @param speedMul Speed multiplier for typing.
 */

		friend void UpdateTyping(TextComponent& txt, float dt, float speedMul);
		friend std::string GetVisibleText(const TextComponent& txt);
	};

/**
 * @brief	Updates the typing animation for the text. 
 *			Advances the number of visible characters based on elapsed time and speed.
 *
 * @param txt Reference to the TextComponent to update.
 * @param dt Delta time since last update.
 * @param speedMul Speed multiplier for typing.
 */

	void UpdateTyping(TextComponent& txt, float dt, float speedMul = 1.f);

/**
 * @brief	Gets the currently visible portion of the text.
 *			Returns a substring of the current line based on the number of visible characters.
 * @param txt Reference to the TextComponent.
 * @return String containing the visible text.
 */

	std::string GetVisibleText(const TextComponent& txt);

/**
 * @brief	Loads dialogue data from a JSON file into a TextComponent.
 *			Reads a JSON file, parses dialogue lines and settings, and applies them to the TextComponent.
 * @param filepath Path to the JSON file.
 * @param textComp Reference to the TextComponent to populate.
 * @return True if loading was successful, false otherwise.
 */

	bool LoadDialogueFromJson(const std::string& path, TextComponent& out);
}