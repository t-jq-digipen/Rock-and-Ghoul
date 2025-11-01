#include "TextSystem.h"

namespace TextSystem
{
	TextComponent::TextComponent() : lines{ "" }, currentLineIndex{ 0 }, textSpeed{ 0.f }, speakerID{ 0 }, fontPath{}, fontSize{ 0 }, isTyping{}, color{} {}
	TextComponent::TextComponent(std::vector<std::string> lines, int currentLineIndex, float textSpeed, int speakerID, std::string fontPath, float fontSize, bool isTyping, Vec3 color)
		: lines{ lines }, currentLineIndex{ currentLineIndex }, textSpeed{ textSpeed }, speakerID{ speakerID }, fontPath{ fontPath }, fontSize{ fontSize }, color{ color } {}

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
		const std::string content = FileIO::ReadFile(filepath);
		if (content.empty()) {
			std::cerr << "Failed to load dialogue from: " << filepath << '\n';
			return false;
		}

		try {
			nlohmann::json j = nlohmann::json::parse(content);

			// Fields with defaults
			std::vector<std::string> lines = j.value("lines", std::vector<std::string>{""});
			if (lines.empty()) lines.push_back("");

			const float textSpeed = j.value("textSpeed", 1.f);
			const int speakerID = j.value("speakerID", 0);
			const std::string fontPath = j.value("fontPath", std::string{});
			const float fontSize = j.value("fontSize", 1.f);

			Vec3 color{ 0.f, 0.f, 0.f };
			color.x = j.value("colorR", 1.f);
			color.y = j.value("colorG", 1.f);
			color.z = j.value("colorB", 1.f);

			// Apply to component
			textComp.SetLines(lines);
			textComp.SetCurrentLineIndex(0);
			textComp.SetTextSpeed(textSpeed);
			textComp.SetSpeakerID(speakerID);
			textComp.SetFontPath(fontPath);
			textComp.SetFontSize(fontSize);
			textComp.SetColor(color);
			textComp.StartTyping(); // begin typewriter effect

			return true;
		}
		catch (const std::exception& e) {
			std::cerr << "Dialogue json parse error (" << filepath << "): " << e.what() << '\n';
			return false;
		}
	}
}