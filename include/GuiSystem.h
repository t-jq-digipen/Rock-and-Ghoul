/* ========================================================
 * @file        GuiSystem.h
 * @author		Manfred Sim | manfred.sim@digipen.edu
 * @date        30 OCT 2025
 *
 * @brief
 *
 * Declares the GuiSystem, an engine subsystem that manages the 
 * in-game editor for UI elements (buttons, textboxes and
 * dialogue boxes).
 *
 * Features:
 * - Toggles on/off with a key press (F8).
 * - Renders a single "GUI Editor" window onto the game screen
 * with selectable panels for each active UI element.
 * - Scales with the game screen, including when ImGui Editor 
 * is active.
 * - Configurations for each UI element type.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include "System.h"
#include "Render.h"
#include "Input.h"
#include "AudioSystem.h"
#include "TextSystem.h"
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

    // baseline (screen-space anchors)
    Vec2 basePos;         // base position for scaling (1920x1080)
    Vec2 baseSize;
    float baseTextScale = 1.f;
};

struct TextBoxConfig {
    std::string text;
    Vec2 pos;
    float scale = 1.0f;
    Colour3 color{ 0, 0, 0 };   //black text
    std::string fontKey = "Marker";
    bool centered = false;      //centering

    // baseline (screen-space anchors)
    Vec2 basePos;
    float baseScale = 1.f;
};

struct DialogueConfig {
    TextSystem::TextComponent* textComp = nullptr;
    TextBoxConfig textBox;
    std::shared_ptr<TextSystem::TextComponent> owned;
    std::string jsonPath;
};


struct Button {
    ButtonConfig cfg;
    bool hovered = false;
    bool pressed = false;
};
class GuiSystem : public ISystem {
public:
    static GuiSystem& Get() {
        static GuiSystem instance;
        return instance;
    }
    void Initialize() override;
    void Update(float dt) override;
    void Shutdown() override;

    void AddButton(const ButtonConfig& cfg);
    void AddTextBox(const TextBoxConfig& cfg);
    static void SetAudioManager(AudioManager* audioMgr);

    void AddDialogue(const DialogueConfig& cfg);
    int AddDialogueFromJson(const std::string& jsonPath, const TextBoxConfig& textBox);

    void SetEditMode(bool isEditing);
    bool GetEditMode() const;


    void SetGameViewRect(const GameViewRect& rect) { m_gameViewRect = rect; }
private:

    std::vector<ButtonConfig> buttons;
    std::vector<TextBoxConfig> textboxes;
    std::vector<DialogueConfig> dialogues;

    static AudioManager* audio;
    Render& render = Render::GetInstance();


    bool editMode{ false };
    int selectedIndex = -1;
    enum class SelectedType { None, Button, TextBox, Dialogue } selectedType = SelectedType::None;

    GameViewRect m_gameViewRect;
};

// inline GuiSystem guiSystem;