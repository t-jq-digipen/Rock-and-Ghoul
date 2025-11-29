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
#include "DataDefs.h"

enum class ButtonShape 
{
    Rectangle, 
    Rounded, 
    Circle 
};

enum class ButtonAction
{
    None,
    LoadLevel1,
    LoadLevel2,
    LoadMainMenu,
    Quit,
    OpenStateWindow,
    ToggleAllButtons,
    ToggleAllTextboxes,
    ToggleAllDialogues,
    GetActionCount
};

enum class SelectedType
{
    None,
    Button,
    TextBox,
    Dialogue
};

enum class UIRenderType
{
    None,
    Shape,
    CustomQuad,
    Image,
    ImageSheet
};

class IUIRender
{
public:
    virtual ~IUIRender() = default;

    virtual std::string Serialize() const = 0;

    virtual UIRenderType GetType() const = 0;

    virtual std::unique_ptr<IUIRender> Clone() const = 0;
};

struct UIShapeRender : public IUIRender
{
    ButtonShape shape = ButtonShape::Rectangle;
    Colour3 color{ 0.3f, 0.3f, 0.3f };

    std::string Serialize() const override {
        using json = nlohmann::json;
        json j;
        std::string shapename = "Rectangle";
        if (shape == ButtonShape::Rounded) { shapename = "Rounded"; }
        else if (shape == ButtonShape::Circle) { shapename = "Circle"; }
        j["ButtonShape"] = shapename;
        j["Color"] = { color.r, color.g, color.b };
        return j.dump();
    }

    UIRenderType GetType() const { return UIRenderType::Shape; }

    std::unique_ptr<IUIRender> Clone() const override {
        return std::make_unique<UIShapeRender>(*this);
    }

    static UIShapeRender DeSerialize(const std::string& data) {

        using json = nlohmann::json;

        json j = json::parse(data);

        UIShapeRender ret;

        ret.shape = ButtonShape::Rectangle;
        if (j["ButtonShape"] == "Circle") { ret.shape = ButtonShape::Circle; }
        if (j["ButtonShape"] == "Rounded") { ret.shape = ButtonShape::Rounded; }
        ret.color = { j["Color"][0], j["Color"][1], j["Color"][2] };

        return ret;
    }

};

struct UICustomQuadRender : public IUIRender
{
    Vec2 v0{0.f,0.f}, v1{ 0.f,0.f }, v2{ 0.f,0.f }, v3{ 0.f,0.f };
    Colour3 color{ 0.3f, 0.3f, 0.3f };

    std::string Serialize() const override {
        using json = nlohmann::json;

        json j;
        j["v0"] = { v0.x, v0.y };
        j["v1"] = { v1.x, v1.y };
        j["v2"] = { v2.x, v2.y };
        j["v3"] = { v3.x, v3.y };

        j["color"] = { color.r, color.g, color.b };

        return j.dump();
    }

    UIRenderType GetType() const { return UIRenderType::CustomQuad; }

    std::unique_ptr<IUIRender> Clone() const override {
        return std::make_unique<UICustomQuadRender>(*this);
    }

    static UICustomQuadRender DeSerialize(const std::string& data) {
        using json = nlohmann::json;
        json j = json::parse(data);

        UICustomQuadRender ret;

        ret.v0 = { j["v0"][0], j["v0"][1] };
        ret.v1 = { j["v1"][0], j["v1"][1] };
        ret.v2 = { j["v2"][0], j["v2"][1] };
        ret.v3 = { j["v3"][0], j["v3"][1] };

        ret.color = { j["color"][0],j["color"][1],j["color"][2] } ;

        return ret;
    }

};

struct UIImageRender : public IUIRender
{
    Image m_image;

    std::string Serialize() const override {
        using json = nlohmann::json;
        json j;
        j["texturePath"] = Render::GetInstance().Get_TexturePath(m_image.textureID);
        return j.dump();
    }

    UIRenderType GetType() const { return UIRenderType::Image; }

    std::unique_ptr<IUIRender> Clone() const override {
        return std::make_unique<UIImageRender>(*this);
    }

    static UIImageRender DeSerialize(const std::string& data) {
        using json = nlohmann::json;
        json j = json::parse(data);

        UIImageRender ret;

        ret.m_image = CreateImage(j["texturePath"]);

        return ret;
    }
};

struct UIImageSheetRender : public IUIRender
{
    Image m_image;

    struct UVs {
        float u0, v0, u1, v1;
    };

    struct FrameIndex {
        unsigned int curr_row = 0;
        unsigned int curr_col = 0;
    };

    std::unordered_map<unsigned int, FrameIndex> frames;

    unsigned int current_frame = 0;
    unsigned int total_rows = 1;
    unsigned int total_cols = 1;

    FrameIndex frame_index;


    std::string Serialize() const override {
        using json = nlohmann::json;
        json j;

        j["texturePath"] = Render::GetInstance().Get_TexturePath(m_image.textureID);

        j["currFrame"]  = current_frame;

        j["currRow"]    = frame_index.curr_row;
        j["currCol"]    = frame_index.curr_col;

        json framesJson = json::object();
        for (auto& [state, index] : frames)
        {
            framesJson[std::to_string(state)] = {
                {"row", index.curr_row},
                {"col", index.curr_col}
            };
        }

        j["frames"] = framesJson;

        return j.dump();
    }

    UIRenderType GetType() const { return UIRenderType::ImageSheet; }

    std::unique_ptr<IUIRender> Clone() const override {
        return std::make_unique<UIImageSheetRender>(*this);
    }

    static UIImageSheetRender DeSerialize(const std::string& data) {
        using json = nlohmann::json;
        json j = json::parse(data);

        UIImageSheetRender ret;

        ret.m_image = CreateImage(j["texturePath"]);
        
        ret.current_frame        = j["currFrame"];

        ret.frame_index.curr_row = j["currRow"];
        ret.frame_index.curr_col = j["currCol"];

        if (j.contains("frames"))
        {
            for (auto& [state, frames] : j["frames"].items())
            {
                FrameIndex frameIDX;
                frameIDX.curr_row = frames["row"];
                frameIDX.curr_col = frames["col"];

                ret.frames[std::stoi(state)] = frameIDX;
            }
        }
        else
        {
            ret.frames[0] = { 0,0 };
        }

        return ret;
    }

};

struct UI_RenderData {
    std::unique_ptr<IUIRender> data = nullptr;

    UI_RenderData() : data{ nullptr } {}

    UI_RenderData(const UI_RenderData& rhs) {
        data = nullptr;
        if (rhs.data) data = rhs.data->Clone();
    }

    UI_RenderData& operator=(const UI_RenderData& rhs) {
        data = nullptr;
        if (rhs.data) data = rhs.data->Clone();

        return *this;
    }

    void SetRenderType(UIRenderType type);

    UIRenderType GetRenderType() const;

    UIShapeRender& GetShapeData();
    const UIShapeRender& GetShapeData() const;

    UICustomQuadRender& GetQuadData();
    const UICustomQuadRender& GetQuadData() const;

    UIImageRender& GetImageData();
    const UIImageRender& GetImageData() const;

    UIImageSheetRender& GetImageSheetData();
    const UIImageSheetRender& GetImageSheetData() const;

    std::string Serialize() const;

    static UI_RenderData DeSerialize(const std::string& data);

};

struct ButtonConfig {
    std::string label;
    Vec2 pos;
    Vec2 size{ 200.f, 100.f };
    //Colour3 color{ 0.3f, 0.3f, 0.3f };

    UI_RenderData m_renderData{};

    float textScale{ 1.f };

    std::function<void()> onClick = nullptr;
    AudioSystem::ButtonType soundType = AudioSystem::ButtonType::PLAY;
    //ButtonShape shape = ButtonShape::Rectangle;
    ButtonAction action = ButtonAction::None;

    // baseline (screen-space anchors)
    Vec2 basePos;         // base position for scaling (1920x1080)
    Vec2 baseSize;
    float baseTextScale{ 1.f };
    bool isVisible = true;
    int ID;


    void DrawButton(bool hovered);
};

struct TextBoxConfig {
    std::string text;
    Vec2 pos;
    float scale{ 1.f };
    Colour3 color{ 0, 0, 0 };   //black text
    std::string fontKey = "Marker";
    bool centered = false;      //centering
    bool isVisible = true;

    // baseline (screen-space anchors)
    Vec2 basePos;
    float baseScale{ 1.f };
    int ID;
};

struct DialogueConfig {
    TextSystem::TextComponent* textComp = nullptr;
    TextBoxConfig textBox;
    std::shared_ptr<TextSystem::TextComponent> owned;
    std::string jsonPath;
    std::string sceneID = "0";
    bool isVisible = true;
    int ID;
};


struct Button {
    ButtonConfig cfg;
    bool hovered = false;
    bool pressed = false;
};

std::string ButtonShapeToString(ButtonShape shape);
ButtonShape StringToButtonShape(const std::string& s);
std::string ButtonActionToString(ButtonAction action);
ButtonAction StringToButtonAction(const std::string& s);

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
    int AddDialogueFromJson(const std::string& jsonPath, const TextBoxConfig& textBox, const std::string& sceneID = "0");

    void SetEditMode(bool isEditing);
    bool GetEditMode() const;


    void SetGameViewRect(const GameViewRect& rect) { m_gameViewRect = rect; }

    // Non-Const Getters
    std::vector<ButtonConfig>& Buttons() { return buttons; }
    std::vector<TextBoxConfig>& TextBoxes() { return textboxes; }
    std::vector<DialogueConfig>& Dialogues() { return dialogues; }

    // Const Getters
    const std::vector<ButtonConfig>& GetButtons() const { return buttons; }
    const std::vector<TextBoxConfig>& GetTextBoxes() const { return textboxes; }
    const std::vector<DialogueConfig>& GetDialogues() const { return dialogues; }
    const SelectedType& GetSelectedType() const { return selectedType; }
    const int& GetSelectedIndex() const { return selectedIndex; }

    void Clear();

    // Button functions
    void BindButtonAction(ButtonConfig& btn);
    void ToggleAllButtons(const ButtonConfig* exclude = nullptr);
    void ToggleAllTextboxes();
    void ToggleAllDialogues();
    void DrawStateWindow(Render& render);
    
    // Helper to get mouse position if ImGui editor is open
    void GetMouseImgui(Vec2& pos);

    // Dialogue helpers
    void SetDialogueLine(DialogueConfig& dlg, int newIndex, bool restartTyping);

private:

    std::vector<ButtonConfig> buttons;
    std::vector<TextBoxConfig> textboxes;
    std::vector<DialogueConfig> dialogues;

    struct DrawItem
    {
        SelectedType type;
        int index;
        int ID;
    };
    std::vector<DrawItem> BuildSortedDrawList() const;

    // ID assignment for new elements (if ID not set)
    int nextUIID = 1;

    static AudioManager* audio;
    Render& render = Render::GetInstance();


    bool editMode{ false };
    int selectedIndex = -1;
    enum class SelectedType selectedType = SelectedType::None;

    GameViewRect m_gameViewRect;

    // state window overlay
    bool stateWindowActive = false;
};

// inline GuiSystem guiSystem;