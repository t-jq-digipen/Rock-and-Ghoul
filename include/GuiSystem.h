/* ========================================================
 * @file        GuiSystem.h
 * @author		Manfred Sim     | manfred.sim@digipen.edu
 * @co-author   Tan Jun Quan    | t.junquan@digipen.edu
 * @date        30 NOV 2025
 *
 * @brief
 *
 * Implements the GuiSystem, which provides a in-game editor
 * for UI elements (inside the ImGui editor). This header file
 * declares the functions and enumerations to be defined in
 * GuiSystem.cpp.
 *
 * Copyright � 2025 DigiPen, All rights reserved.
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
    ToggleAllDisplays,
    LoadSettings,
    LoadControls,
    GetActionCount
};

enum class SelectedType
{
    None,
    Button,
    TextBox,
    Dialogue,
    Display
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

/**
 * @brief Serializes the UIShapeRender to a JSON string.
 * @return Serialized JSON string.
 */

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

/**
 * @brief Gets the render type of the UIShapeRender.
 * @return UIRenderType::Shape.
 */

    UIRenderType GetType() const { return UIRenderType::Shape; }

/**
 * @brief Clones the UIShapeRender object.
 * @return Unique pointer to the cloned UIShapeRender.
 */

    std::unique_ptr<IUIRender> Clone() const override {
        return std::make_unique<UIShapeRender>(*this);
    }

/**
 * @brief Deserializes a UIShapeRender from a JSON string.
 * @param data JSON string.
 * @return Deserialized UIShapeRender object.
*/

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
    Vec2 v0{ 0.f,0.f }, v1{ 0.f,0.f }, v2{ 0.f,0.f }, v3{ 0.f,0.f };
    Colour3 color{ 0.3f, 0.3f, 0.3f };

/**
 * @brief Serializes the UICustomQuadRender to a JSON string.
 * @return Serialized JSON string.
 */

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

/**
 * @brief Gets the render type of the UICustomQuadRender.
 * @return UIRenderType::CustomQuad.
 */

    UIRenderType GetType() const { return UIRenderType::CustomQuad; }

/**
 * @brief Clones the UICustomQuadRender object.
 * @return Unique pointer to the cloned UICustomQuadRender.
 */

    std::unique_ptr<IUIRender> Clone() const override {
        return std::make_unique<UICustomQuadRender>(*this);
    }

/**
 * @brief Deserializes a UICustomQuadRender from a JSON string.
 * @param data JSON string.
 * @return Deserialized UICustomQuadRender object.
 */

    static UICustomQuadRender DeSerialize(const std::string& data) {
        using json = nlohmann::json;
        json j = json::parse(data);

        UICustomQuadRender ret;

        ret.v0 = { j["v0"][0], j["v0"][1] };
        ret.v1 = { j["v1"][0], j["v1"][1] };
        ret.v2 = { j["v2"][0], j["v2"][1] };
        ret.v3 = { j["v3"][0], j["v3"][1] };

        ret.color = { j["color"][0],j["color"][1],j["color"][2] };

        return ret;
    }

};

struct UIImageRender : public IUIRender
{
    Image m_image;

/**
 * @brief Serializes the UIImageRender to a JSON string.
 * @return Serialized JSON string.
 */

    std::string Serialize() const override {
        using json = nlohmann::json;
        json j;
        j["texturePath"] = Render::GetInstance().Get_TexturePath(m_image.textureID);
        return j.dump();
    }

/**
 * @brief Gets the render type of the UIImageRender.
 * @return UIRenderType::Image.
 */

    UIRenderType GetType() const { return UIRenderType::Image; }

/**
 * @brief Clones the UIImageRender object.
 * @return Unique pointer to the cloned UIImageRender.
 */

    std::unique_ptr<IUIRender> Clone() const override {
        return std::make_unique<UIImageRender>(*this);
    }

/**
 * @brief Deserializes a UIImageRender from a JSON string.
 * @param data JSON string.
 * @return Deserialized UIImageRender object.
 */

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

/**
 * @brief Serializes the UIImageSheetRender to a JSON string.
 * @return Serialized JSON string.
 */

    std::string Serialize() const override {
        using json = nlohmann::json;
        json j;

        j["texturePath"] = Render::GetInstance().Get_TexturePath(m_image.textureID);

        j["totalRows"] = total_rows;
        j["totalCols"] = total_cols;

        j["currFrame"] = current_frame;

        j["currRow"] = frame_index.curr_row;
        j["currCol"] = frame_index.curr_col;

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

/**
 * @brief Gets the render type of the UIImageSheetRender.
 * @return UIRenderType::ImageSheet.
 */

    UIRenderType GetType() const { return UIRenderType::ImageSheet; }

/**
 * @brief Clones the UIImageSheetRender object.
 * @return Unique pointer to the cloned UIImageSheetRender.
 */

    std::unique_ptr<IUIRender> Clone() const override {
        return std::make_unique<UIImageSheetRender>(*this);
    }

/**
 * @brief Deserializes a UIImageSheetRender from a JSON string.
 * @param data JSON string.
 * @return Deserialized UIImageSheetRender object.
 */

    static UIImageSheetRender DeSerialize(const std::string& data) {
        using json = nlohmann::json;
        json j = json::parse(data);

        UIImageSheetRender ret;

        ret.m_image = CreateImage(j["texturePath"]);

        ret.current_frame = j["currFrame"];

        ret.total_rows = j["totalRows"];
        ret.total_cols = j["totalCols"];

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

/**
 * @brief Sets the current frame for the image sheet.
 * @param frame Frame index to set.
 * @return True if frame was set, false otherwise.
 */

    bool SetFrame(unsigned int frame) {
        if (current_frame == frame) { return false; }

        current_frame = frame;
        if (frames.find(frame) == frames.end()) {
            std::cerr << "frame does not exist in this UI display\n";
            return false;
        }
        frame_index = frames[current_frame];
        return true;
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

/**
 * @brief Sets the render type for the UI_RenderData.
 * @param type The render type to set.
 */

    void SetRenderType(UIRenderType type);

/**
 * @brief Gets the render type of the UI_RenderData.
 * @return The current UIRenderType.
 */

    UIRenderType GetRenderType() const;

/**
 * @brief Gets the shape render data.
 * @return Reference to UIShapeRender.
 */

    UIShapeRender& GetShapeData();

/**
 * @brief Gets the shape render data (const).
 * @return Const reference to UIShapeRender.
 */

    const UIShapeRender& GetShapeData() const;

/**
 * @brief Gets the custom quad render data.
 * @return Reference to UICustomQuadRender.
 */

    UICustomQuadRender& GetQuadData();

/**
 * @brief Gets the custom quad render data (const).
 * @return Const reference to UICustomQuadRender.
 */

    const UICustomQuadRender& GetQuadData() const;

/**
 * @brief Gets the image render data.
 * @return Reference to UIImageRender.
 */

    UIImageRender& GetImageData();

/**
 * @brief Gets the image render data (const).
 * @return Const reference to UIImageRender.
 */

    const UIImageRender& GetImageData() const;

/**
 * @brief Gets the image sheet render data.
 * @return Reference to UIImageSheetRender.
 */

    UIImageSheetRender& GetImageSheetData();

/**
 * @brief Gets the image sheet render data (const).
 * @return Const reference to UIImageSheetRender.
 */

    const UIImageSheetRender& GetImageSheetData() const;

/**
 * @brief Serializes the UI_RenderData to a JSON string.
 * @return Serialized JSON string.
 */

    std::string Serialize() const;

/**
 * @brief Deserializes a UI_RenderData from a JSON string.
 * @param data JSON string.
 * @return Deserialized UI_RenderData object.
 */

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

    std::string actionName{"[]"};
    json actionParams{ "[]" };

    // baseline (screen-space anchors)
    Vec2 basePos;         // base position for scaling (1920x1080)
    Vec2 baseSize;
    float baseTextScale{ 1.f };
    bool isVisible = true;
    int ID = 0;

/**
 * @brief Draws the button using the current configuration.
 * @param hovered Whether the button is currently hovered.
 */

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
    int ID = 0;
};

struct DialogueConfig {
    TextSystem::TextComponent* textComp = nullptr;
    TextBoxConfig textBox;
    std::shared_ptr<TextSystem::TextComponent> owned;
    std::string jsonPath;
    std::string sceneID = "0";
    bool isVisible = true;
    int ID = 0;

    // New variables
    float textSpeed = 0.f; // text typing speed
    float wrapWidth = 0.f; // max width of line before next line
    bool customSpeaker = false; // for setting custom speaker
    std::string customSpeakerName = "Beberd";

    Vec2 speakerNameBasePos;
    float speakerNameBaseScale{ 1.2f };
    Colour3 speakerNameColor = { 0, 0, 0 };
    bool speakerNameCentered = false;
};

struct DisplayConfig
{
    Vec2 pos;
    Vec2 size;
    Vec2 basePos;
    Vec2 baseSize;
    bool centered = false;
    UI_RenderData m_renderData{};
    bool isVisible = true;
    int ID = 0;
};

struct Button {
    ButtonConfig cfg;
    bool hovered = false;
    bool pressed = false;
};

struct UIGroup
{
    int groupID = 0;
    std::string groupName;
    std::vector<int> memberIDs;
};

/**
 * @brief Converts a ButtonShape enum value to its string representation.
 * @param shape The ButtonShape value.
 * @return String representation of the ButtonShape.
 */

std::string ButtonShapeToString(ButtonShape shape);

/**
 * @brief Converts a string to a ButtonShape enum value.
 * @param str The string representation of the ButtonShape.
 * @return Corresponding ButtonShape value.
 */

ButtonShape StringToButtonShape(const std::string& s);

/**
 * @brief Converts a ButtonAction enum value to its string representation.
 * @param action The ButtonAction value.
 * @return String representation of the ButtonAction.
 */

std::string ButtonActionToString(ButtonAction action);

/**
 * @brief Converts a string to a ButtonAction enum value.
 * @param str The string representation of the ButtonAction.
 * @return Corresponding ButtonAction value.
 */

ButtonAction StringToButtonAction(const std::string& s);

class GuiSystem : public ISystem {
public:
    static GuiSystem& Get() {
        static GuiSystem instance;
        return instance;
    }

/**
 * @brief   Initializes the GUI system and registers all button actions.
 *          Clears all UI containers, resets ID allocation and registers:
 *          - Scene loading actions (LoadScene, LoadSettings, LoadControls)
 *          - Quit the application
 *          - Volume up/down + save settings
 *          - Controls page paging actions
 */

    void Initialize() override;

/**
 * @brief       Main per-frame update for the GUI system.
 *              Functions:
 *              - Handles dialogue progression on SPACE (when not editing)
 *              - Converts mouse coords into game-view space
 *              - Builds draw order and render all UI elements (buttons, textboxes, dialogues, displays)
 *              - Applies transition overlay and state window rendering
 * @param dt    Delta time in seconds since last frame.
 */

    void Update(float dt) override;

/**
 * @brief   Shuts down the GUI system and clears all managed UI elements. 
            Resets the next UI ID to 1.
 */

    void Shutdown() override;

/**
 * @brief           Sets the shared AudioManager pointer used by the GUI system.
 * @param audioMgr  Pointer to the AudioManager instance.
 */

    static void SetAudioManager(AudioManager* audioMgr);

/**
 * @brief       Adds a new button to the GUI system with a unique ID.
 * @param cfg   The button configuration to add.
 */

    void AddButton(const ButtonConfig& cfg);

/**
 * @brief       Adds a new textbox to the GUI system with a unique ID.
 * @param cfg   The textbox configuration to add.
 */

    void AddTextBox(const TextBoxConfig& cfg);

/**
 * @brief       Adds a new dialogue to the GUI system with a unique ID.
 * @param cfg   The dialogue configuration to add.
 */

    void AddDialogue(const DialogueConfig& cfg);

/**
 * @brief           Helper to spawn a DialogueConfig from a JSON dialogue file and textbox template.
 * @param jsonPath  Path to the dialogue JSON file.
 * @param textBox   Template textbox configuration (position, scale, etc.).
 * @param sceneID   Scene ID within the dialogue file (defaults to "0").
 * @return          Index of the newly added dialogue in dialogues container, or -1 on failure.
 */

    int AddDialogueFromJson(const std::string& jsonPath, const TextBoxConfig& textBox, const std::string& sceneID = "0");

/**
 * @brief       Adds a new display to the GUI system with a unique ID.
 * @param cfg   The display configuration to add.
 */

    void AddDisplay(const DisplayConfig& cfg);

/**
 * @brief           Enables or disables GUI edit mode.
 * @param isEditing True to enable edit mode, false to disable.
 */

    void SetEditMode(bool isEditing);

/**
 * @brief   Returns whether GUI is currently in edit mode.
 * @return  True if editing, false otherwise.
 */

    bool GetEditMode() const;

/**
 * @brief       Set the game view rectangle.
 * @param rect  The game view rectangle to be rendered.
 */

    void SetGameViewRect(const GameViewRect& rect) { m_gameViewRect = rect; }

    // Non-Const Getters

/**
 * @brief   Get a reference to the current list of buttons (ButtonConfig objects).
 * @return  The reference.
 */

    std::vector<ButtonConfig>& Buttons() { return buttons; }

/**
 * @brief   Get a reference to the current list of textboxes (TextBoxConfig objects).
 * @return  The reference.
 */

    std::vector<TextBoxConfig>& TextBoxes() { return textboxes; }

/**
 * @brief   Get a reference to the current list of dialogues (DialogueConfig objects).
 * @return  The reference.
 */

    std::vector<DialogueConfig>& Dialogues() { return dialogues; }

/**
 * @brief   Get a reference to the current list of displays (DisplayConfig objects).
 * @return  The reference.
 */

    std::vector<DisplayConfig>& Displays() { return displays; }

    // Const Getters

/**
 * @brief   Get a const reference to the current list of buttons (ButtonConfig objects).
 * @return  The const reference.
 */

    const std::vector<ButtonConfig>& GetButtons() const { return buttons; }

/**
 * @brief   Get a const reference to the current list of textboxes (TextBoxConfig objects).
 * @return  The const reference.
 */

    const std::vector<TextBoxConfig>& GetTextBoxes() const { return textboxes; }

/**
 * @brief   Get a const reference to the current list of dialogues (DialogueConfig objects).
 * @return  The const reference.
 */

    const std::vector<DialogueConfig>& GetDialogues() const { return dialogues; }

/**
 * @brief   Get a const reference to the current list of displays (DisplayConfig objects).
 * @return  The const reference.
 */

    const std::vector<DisplayConfig>& GetDisplays() const { return displays; }

/**
 * @brief   Get a reference to the type of the selected UI element.
 * @return  The type of the selected UI element.
 */

    const SelectedType& GetSelectedType() const { return selectedType; }

/**
 * @brief   Get a reference to the index of the selected UI element.
 * @return  The index of the selected UI element.
 */

    const int& GetSelectedIndex() const { return selectedIndex; }

/**
 * @brief   Clears all managed UI elements. Resets the next UI ID to 1.
 */

    void Clear();

    // Button functions

/**
 * @brief       Binds built-in click behaviors to a button based on its ButtonAction.
 * @param btn   The button configuration to modify.
 */

    void BindButtonAction(ButtonConfig& btn);

/**
 * @brief           Toggles visibility of all buttons except an optional excluded one.
 *                  If all considered buttons are visible, hides them; otherwise shows them.
 * @param exclude   Optional button pointer to ignore when toggling.
 */

    void ToggleAllButtons(const ButtonConfig* exclude = nullptr);

/**
 * @brief Toggles visibility of all textboxes at once.
 */

    void ToggleAllTextboxes();

/**
 * @brief Toggles visibility of all dialogues at once.
 */

    void ToggleAllDialogues();

/**
 * @brief Toggles visibility of all displays at once.
 */

    void ToggleAllDisplays();

/**
 * @brief           Draws the state window overlay UI (e.g. "Choose Your State" menu).
 *                  This overlay is drawn in screen space, including its own buttons and close control.
 * @param render    Reference to the Render instance to use.
 */

    void DrawStateWindow(Render& render);

/**
 * @brief       Converts the given mouse position to game-view coordinates when ImGui is active.
 *              If mouse is outside the game view rect, moves it off-screen so UI hit tests ignore it.
 * @param mouse Reference to mouse position to modify in-place.
 */

    void GetMouseImgui(Vec2& pos);

/**
 * @brief               Sets the current line of a dialogue text component, optionally restarting typing.
 *                      Bounds-checks the requested index, stops typing if currently typing, and sets
 *                      the new current line index. Optionally starts typing again from the new line.
 * @param dlg           Dialogue configuration whose text component should be updated.
 * @param newIndex      Desired line index within the dialogue lines.
 * @param restartTyping True to restart the typewriter effect; false to leave it stopped.
 */

    void SetDialogueLine(DialogueConfig& dlg, int newIndex, bool restartTyping);

    // Action registry
    // make actions a parameterized function instead to take string,
    // can save params into a json c:
    using ActionFunction = std::function<void(const json&)>;

/**
 * @brief       Registers an action callback with a string name.
 * @param name  Name of the action.
 * @param func  Callback accepting const json& parameters.
 */

    void RegisterAction(const std::string& name, ActionFunction func);

/**
 * @brief           Invokes a previously registered action by name.
 *                  Logs a warning to stderr if the action is not found or throws an exception.
 * @param name      Name of the action to invoke.
 * @param params    JSON parameters to pass to the registered callback.
 * @return true     If the action was found and invoked (even if it threw).
 * @return false    If the action name was not found.
 */

    bool CallAction(const std::string& name, const nlohmann::json& params);

/**
 * @brief   Gets a map of all action registries.
 * @return  The map of action registries.
 */

    std::unordered_map<std::string, ActionFunction>& GetActionMap() { return actionRegistry; }

/**
 * @brief   Reseeds the next UI element ID from the maximum existing ID plus one.
 *          Scans all buttons, textboxes, dialogues and displays.
 *          Used when loading scenes to keep IDs globally unique.
 */

    void ReseedNextUIID();

/**
 * @brief       Returns a reference to a UIGroup by name.
 * @param name  Group name to search for.
 * @return      UIGroup& Reference to the matching group.
 */

    std::vector<UIGroup>& GetGroups() { return m_groups; }

/**
 * @brief       Add a group of UI elements to a list of groups.
 * @param group Group to be added/
 */

    void AddGroup(const UIGroup& group) { m_groups.push_back(group); }

/**
 * @brief       Returns a reference to a UIGroup by name.
 * @param name  Group name to search for.
 * @return      UIGroup& Reference to the matching group.
 */

    UIGroup& GetGroup(const std::string& name);

/**
 * @brief           Sets the current frame (state) of an ImageSheet-based display by UI ID.
 *                  Only valid for Display elements with UIRenderType::ImageSheet.
 *                  Logs an error if the ID is not an ImageSheet display.
 * @param ID        The display's unique UI ID.
 * @param frame     Desired frame/state ID in the sheet.
 * @return true     If the frame was updated successfully.
 * @return false    If display not found or is not ImageSheet.
 */

    bool SetDisplaySheetState(int uiID, unsigned int frame);

    bool GetDisplayQuad(int ID, Vec2& v0, Vec2& v1, Vec2& v2, Vec2& v3);

    bool SetDisplayQuad(int ID, Vec2 v0, Vec2 v1, Vec2 v2, Vec2 v3);

private:

    std::vector<ButtonConfig> buttons;
    std::vector<TextBoxConfig> textboxes;
    std::vector<DialogueConfig> dialogues;
    std::vector<DisplayConfig> displays;

    std::vector<UIGroup> m_groups;

    // a registry to hold functions, keyed by function name
    std::unordered_map<std::string, ActionFunction> actionRegistry;

    struct DrawItem
    {
        SelectedType type;
        int index;
        int ID;
    };

/**
 * @brief   Builds a single draw list of all UI elements sorted by ascending ID.
 *          The resulting DrawItem vector is used in Update() to render UI in a stable order
 *          across all UI types: Buttons, TextBoxes, Dialogues and Displays.
 * @return  Sorted list of drawable UI references.
 */

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