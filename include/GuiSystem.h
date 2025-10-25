#pragma once
#include "System.h"
#include "Render.h"
#include "Input.h"
#include "AudioSystem.h"
#include <functional>
#include <vector>
#include <string>

enum class ButtonShape {
    Rectangle,
    Circle,
    Rounded
};

struct ButtonConfig {
    std::string label;
    Vec2 pos;
    Vec2 size{ 200.f, 100.f };
    Colour3 color{ 0.3f, 0.3f, 0.3f };
    std::function<void()> onClick = nullptr;
    AudioSystem::ButtonType soundType = AudioSystem::ButtonType::PLAY;
    ButtonShape shape = ButtonShape::Rectangle;
    float textScale = 1.0f;
};

struct TextBoxConfig {
    std::string text;
    Vec2 pos;
    float scale = 1.0f;
    Colour3 color{ 0, 0, 0 };   //black text
    bool centered = false;      //centering
};


struct Button {
    ButtonConfig cfg;
    bool hovered = false;
    bool pressed = false;
};
class GuiSystem : public ISystem {
public:
    void Initialize() override;
    void Update(float dt) override;
    void Shutdown() override;

    void AddButton(const ButtonConfig& cfg);
    void AddTextBox(const TextBoxConfig& cfg);
    static void SetAudioManager(AudioManager* audioMgr);

private:
    std::vector<ButtonConfig> buttons;
    std::vector<TextBoxConfig> textboxes;
    static AudioManager* audio;
    Render& render = Render::GetInstance();


    bool editMode = false;
    int selectedIndex = -1;
    enum class SelectedType { None, Button, TextBox } selectedType = SelectedType::None;
};
