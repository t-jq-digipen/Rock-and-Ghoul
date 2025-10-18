#pragma once
#include <string>
#include <vector>
#include <Remath.h>

namespace Text
{
	class TextComponent {
		std::vector<std::string> lines;
		int currentLineIndex{};
		float charsPerSecond{};
		int speakerID{};
		std::string fontPath;
		int fontSize{ 0 };
		Vec3 color;

	public:
		// Default constructor
		TextComponent();
		TextComponent(std::vector<std::string> lines, int currentLineIndex, float charsPerSecond, int speakerID, std::string fontPath, int fontSize, Vec3 color);
		TextComponent(const TextComponent& other) = default;

		// GETTERS
		const std::vector<std::string>&	GetLines() const noexcept;
		int	GetCurrentLineIndex() const noexcept;
		float GetCharsPerSecond() const noexcept;
		int GetSpeakerID() const noexcept;
		const std::string& GetFontPath() const noexcept;
		int GetFontSize() const noexcept;
		const Vec3& GetColor() const noexcept;

		// SETTERS
		void SetLines(const std::vector<std::string>& script);
		void SetCurrentLineIndex(int index);
		void SetCharsPerSecond(float speed);
		void SetSpeakerID(int id);
		void SetFontPath(const std::string& path);
		void SetFontSize(int size);
		void SetColor(Vec3 const& colorVec);
	};
}

