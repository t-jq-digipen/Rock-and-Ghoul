#include "TextSystem.h"

namespace TextSystem
{
	TextComponent::TextComponent() : lines{ "" }, currentLineIndex{0}, charsPerSecond{0.f}, speakerID{0}, fontPath{}, fontSize{0}, color{} {}
	TextComponent::TextComponent(std::vector<std::string> lines, int currentLineIndex, float charsPerSecond, int speakerID, std::string fontPath, int fontSize, Vec3 color)
		: lines{ lines }, currentLineIndex{ currentLineIndex }, charsPerSecond{ charsPerSecond }, speakerID{ speakerID }, fontPath{ fontPath }, fontSize{ fontSize }, color{ color } {}

	// GETTERS
	const std::vector<std::string>& TextComponent::GetLines() const noexcept { return lines; }
	int	TextComponent::GetCurrentLineIndex() const noexcept { return currentLineIndex; }
	float TextComponent::GetCharsPerSecond() const noexcept { return charsPerSecond; }
	int TextComponent::GetSpeakerID() const noexcept { return speakerID; }
	const std::string& TextComponent::GetFontPath() const noexcept { return fontPath; }
	int TextComponent::GetFontSize() const noexcept { return fontSize; }
	const Vec3& TextComponent::GetColor() const noexcept { return color; }
	const std::string& TextComponent::GetCurrentLine(int index) const noexcept { return lines[index]; };

	// SETTERS
	void TextComponent::SetLines(const std::vector<std::string>& script) { lines = script; }
	void TextComponent::SetCurrentLineIndex(int index) { currentLineIndex = index; }
	void TextComponent::SetCharsPerSecond(float speed) { charsPerSecond = speed; }
	void TextComponent::SetSpeakerID(int id) { speakerID = id; }
	void TextComponent::SetFontPath(const std::string& path) { fontPath = path; }
	void TextComponent::SetFontSize(int size) { fontSize = size; }
	void TextComponent::SetColor(Vec3 const& colorVec) { color = colorVec; }
}