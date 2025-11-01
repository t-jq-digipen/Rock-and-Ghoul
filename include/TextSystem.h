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
		int speakerID{};
		std::string fontPath;
		float fontSize{ 0 };
		Vec3 color;

		// while typing (not serialized)
		bool isTyping{ false };
		float timeElapsed{ 0.f };
		int visibleChars{ 0 };

	public:
		// Default constructor
		TextComponent();
		// Parameterized constructor
		TextComponent(std::vector<std::string> lines, int currentLineIndex,
			float textSpeed, int speakerID, std::string fontPath, float fontSize, bool isTyping, Vec3 color);
		// Copy constructor
		TextComponent(const TextComponent& other) = default;

		// GETTERS
		const std::vector<std::string>& GetLines() const noexcept;
		int	GetCurrentLineIndex() const noexcept;
		float GetTextSpeed() const noexcept;
		int GetSpeakerID() const noexcept;
		const std::string& GetFontPath() const noexcept;
		float GetFontSize() const noexcept;
		bool CheckIfTyping() const noexcept;
		const Vec3& GetColor() const noexcept;
		const std::string& GetCurrentLine(int index) const noexcept;

		// SETTERS
		void SetLines(const std::vector<std::string>& script);
		void SetCurrentLineIndex(int index);
		void SetTextSpeed(float speed);
		void SetSpeakerID(int id);
		void SetFontPath(const std::string& path);
		void SetFontSize(float size);
		void SetIsTyping(bool isType);
		void SetColor(Vec3 const& colorVec);

		// TYPE TEXT CALL FUNCTIONS
		void StartTyping() { isTyping = true; timeElapsed = 0.f; visibleChars = 0; }
		void StopTyping() { isTyping = false; visibleChars = static_cast<int>(GetCurrentLine(GetCurrentLineIndex()).size()); }

		// FRIENDS !!!
		friend void UpdateTyping(TextComponent& txt, float dt, float speedMul);
		friend std::string GetVisibleText(const TextComponent& txt);
	};

	// Helpers (for automatically "typing" each line)
	void UpdateTyping(TextComponent& txt, float dt, float speedMul = 1.f);
	std::string GetVisibleText(const TextComponent& txt);

	// Get Dialogue From Json
	bool LoadDialogueFromJson(const std::string& path, TextComponent& out);
}