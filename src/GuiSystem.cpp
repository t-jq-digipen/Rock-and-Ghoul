/* ========================================================
 * @file        GuiSystem.cpp
 * @author      Manfred Sim | manfred.sim@digipen.edu
 * @co-author	Tan Jun Quan | t.junquan@digipen.edu
 * @date        30 OCT 2025
 *
 * @brief
 *
 * Implements the GuiSystem, which provides a in-game editor 
 * for UI elements (inside the ImGui editor). This source file
 * defines rendering, interaction, actions and utility functions
 * for:
 * - Buttons
 * - Textboxes
 * - Dialogues
 * - Displays
 * 
 * It also implements the following:
 * - Global UI ID management
 * - Settings page (volume bar + save settings)
 * - Controls page ("How to Play")
 * - State window overlay (for jumping to different scenes)
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "GuiSystem.h"
#include "CoreEngine.h"
#include "glapp.h"
#ifndef NO_IMGUI
#include "ImGuiSystem.h"
#endif
#include "SceneManager.h"
#include <FilePath.h>

// Call Audio Manager for GuiSystem
AudioManager* GuiSystem::audio = nullptr;

// Settings
// Master Volume (min 0.f, max 1.f)
static float settingsVolume = 1.f;

// Step size for volume up/down actions
static const float volStep = 0.05f;

// ID for the display ID as loaded from the settings scene
static const int settingsBarFillID = 7; // to match display ID in settings.json

// Maximum width of the settings volume bar fill
static const float settingsBarMaxW = 1000.f;

// Flag to sync settings volume bar fill size after scene loads
bool settingsNeedSync = false;
bool showFPS = false;

// ==============================
// UI_RenderData & variants
// ==============================


/**
 * @brief       Sets the render type of the selected UI element.
 * @param type  The UI render type to be determined.
 */
void UI_RenderData::SetRenderType(UIRenderType type) {
    switch (type)
    {
    case UIRenderType::Shape:
        data = std::make_unique<UIShapeRender>();
        break;

    case UIRenderType::CustomQuad:
        data = std::make_unique<UICustomQuadRender>();
        break;

    case UIRenderType::Image:
        data = std::make_unique<UIImageRender>();
        break;

    case UIRenderType::ImageSheet:
        data = std::make_unique<UIImageSheetRender>();
        break;

    default:
        break;
    }
}

/**
 * @brief   Returns the current render variant type of a UI element.
 * @return  The UIRenderType, or UIRenderType::None if no data.
 */

UIRenderType UI_RenderData::GetRenderType() const {
    if (!data) { return UIRenderType::None; }
    return data->GetType();
}

/**
 * @brief   The accessor for shape render data.
 * @return  The UIShapeRender& reference to the underlying shape render data.
 */

UIShapeRender& UI_RenderData::GetShapeData() {
    if (data->GetType() != UIRenderType::Shape) { throw std::runtime_error("trying to get shape from ui button but is not shape"); }
    return *static_cast<UIShapeRender*>(data.get());
}

/**
 * @brief   The const accessor for shape render data.
 * @return  The UIShapeRender& const reference to the underlying shape render data.
 */

const UIShapeRender& UI_RenderData::GetShapeData() const {
    if (data->GetType() != UIRenderType::Shape) { throw std::runtime_error("trying to get shape from ui button but is not shape"); }
    return *static_cast<const UIShapeRender*>(data.get());
}

/**
 * @brief   The accessor for custom quad render data.
 * @return  The UICustomQuadRender& reference to the underlying quad render data.
 */

UICustomQuadRender& UI_RenderData::GetQuadData() {
    if (data->GetType() != UIRenderType::CustomQuad) { throw std::runtime_error("trying to get shape from ui button but is not shape"); }
    return *static_cast<UICustomQuadRender*>(data.get());
}

/**
 * @brief   The const accessor for custom quad render data.
 * @return  The UICustomQuadRender& const reference to the underlying quad render data.
 */

const UICustomQuadRender& UI_RenderData::GetQuadData() const {
    if (data->GetType() != UIRenderType::CustomQuad) { throw std::runtime_error("trying to get shape from ui button but is not shape"); }
    return *static_cast<const UICustomQuadRender*>(data.get());
}

/**
 * @brief   The accessor for image render data.
 * @return  The UIImageRender& reference to the underlying image render data.
 */

UIImageRender& UI_RenderData::GetImageData() {
    if (data->GetType() != UIRenderType::Image) { throw std::runtime_error("trying to get shape from ui button but is not shape"); }
    return *static_cast<UIImageRender*>(data.get());
}

/**
 * @brief   The const accessor for image render data.
 * @return  The UIImageRender& const reference to the underlying image render data.
 */

const UIImageRender& UI_RenderData::GetImageData() const {
    if (data->GetType() != UIRenderType::Image) { throw std::runtime_error("trying to get shape from ui button but is not shape"); }
    return *static_cast<const UIImageRender*>(data.get());
}

/**
 * @brief   The accessor for image sheet render data.
 * @return  The UIImageSheetRender& reference to the underlying image sheet render data.
 */

UIImageSheetRender& UI_RenderData::GetImageSheetData() {
    if (data->GetType() != UIRenderType::ImageSheet) { throw std::runtime_error("trying to get shape from ui button but is not shape"); }
    return *static_cast<UIImageSheetRender*>(data.get());
}

/**
 * @brief   The const accessor for image sheet render data.
 * @return  The UIImageSheetRender& const reference to the underlying image sheet render data.
 */

const UIImageSheetRender& UI_RenderData::GetImageSheetData() const {
    if (data->GetType() != UIRenderType::ImageSheet) { throw std::runtime_error("trying to get shape from ui button but is not shape"); }
    return *static_cast<const UIImageSheetRender*>(data.get());
}

/**
 * @brief   Serializes the current render data into a JSON string.
 * @return  JSON string representation of the render data, or "{}" if none.
 */

std::string UI_RenderData::Serialize() const {
    using json = nlohmann::json;
    json j;

    switch (GetRenderType())
    {
    case UIRenderType::Shape:      j["type"] = "shape";      break;
    case UIRenderType::CustomQuad: j["type"] = "quad";       break;
    case UIRenderType::Image:      j["type"] = "image";      break;
    case UIRenderType::ImageSheet: j["type"] = "imagesheet"; break;
    case UIRenderType::None:
    default:
        return "{}";
    }

    j["data"] = data->Serialize();
    return j.dump();
}

/**
 * @brief           Deserializes the current render data from a JSON string previously 
                    produced by Serialize().
 * @param dataStr   The JSON string containing type and data.
 * @return          JSON string representation of the render data, or "{}" if none.
 */

UI_RenderData UI_RenderData::DeSerialize(const std::string& dataStr) {
    using json = nlohmann::json;
    json j = json::parse(dataStr);

    UI_RenderData ret{};
    if (j.contains("type"))
    {
        std::string type = j["type"];
        if (type == "shape") {
            ret.data = std::make_unique<UIShapeRender>(UIShapeRender::DeSerialize(j["data"]));
        }
        else if (type == "quad") {
            ret.data = std::make_unique<UICustomQuadRender>(UICustomQuadRender::DeSerialize(j["data"]));
        }
        else if (type == "image") {
            ret.data = std::make_unique<UIImageRender>(UIImageRender::DeSerialize(j["data"]));
        }
        else if (type == "imagesheet") {
            ret.data = std::make_unique<UIImageSheetRender>(UIImageSheetRender::DeSerialize(j["data"]));
        }
    }
    return ret;
}

// ==============================
// ButtonConfig draw
// ==============================

/**
 * @brief           Renders the button using its current render data and hover state.
 *
 *                  The specific draw behavior depends on the ButtonConfig::m_renderData type:
 *                  - Shape: rectangle, circle, or rounded rectangle
 *                  - CustomQuad: arbitrary quad with color
 *                  - Image: textured quad
 *                  - ImageSheet: sub-rectangle of a spritesheet
 * @param hovered   True if the mouse is currently hovering the button.
 */

void ButtonConfig::DrawButton(bool hovered) {
    Render& render = Render::GetInstance();

    switch (m_renderData.GetRenderType())
    {
    case UIRenderType::Shape:
    {
        UIShapeRender data = m_renderData.GetShapeData();

        Colour3 drawColor = hovered
            ? Colour3{ data.color.r + 0.2f, data.color.g + 0.2f, data.color.b + 0.2f }
        : data.color;

        switch (data.shape) {
        case ButtonShape::Rectangle:
            render.DrawRect(pos, size.x, size.y, drawColor);
            break;
        case ButtonShape::Circle:
        {
            float radius = std::min(size.x, size.y) * 0.5f;
            render.DrawCircle(pos, radius, drawColor, 48);
            break;
        }
        case ButtonShape::Rounded:
        {
            float corner = std::min(size.x, size.y) * 0.5f;
            render.DrawRoundedRect(pos, size.x, size.y, corner, drawColor);
            break;
        }
        }
    }
    break;

    case UIRenderType::CustomQuad:
    {
        UICustomQuadRender data = m_renderData.GetQuadData();
        Colour3 drawColor = hovered
            ? Colour3{ data.color.r + 0.2f, data.color.g + 0.2f, data.color.b + 0.2f }
        : data.color;

        render.DrawCustomQuad(data.v0, data.v1, data.v2, data.v3, drawColor);
    }
    break;

    case UIRenderType::Image:
    {
        UIImageRender data = m_renderData.GetImageData();
        float brightness = hovered ? .25f : 0.f;
        render.DrawImageRotated(data.m_image, pos.x, pos.y, size.x, size.y, 0.f, brightness);
    }
    break;

    case UIRenderType::ImageSheet:
    {
        UIImageSheetRender data = m_renderData.GetImageSheetData();

        float brightness = hovered ? .25f : 0.f;

        int tileW = data.m_image.widthPixels / data.total_cols;
        int tileH = data.m_image.heightPixels / data.total_rows;

        int col = data.frame_index.curr_col;
        int row = data.frame_index.curr_row;
        row = data.total_rows - 1 - row;

        float u0 = static_cast<float>(col * tileW);
        float v0 = static_cast<float>(row * tileH);
        float u1 = static_cast<float>(u0 + tileW);
        float v1 = static_cast<float>(v0 + tileH);

        render.DrawImageSub(data.m_image, pos.x, pos.y, size.x, size.y, u0, v0, u1, v1, 0.f, brightness);
    }
    break;

    default:
        break;
    }
}

// ==============================
// GuiSystem impl
// ==============================

/**
 * @brief           Sets the shared AudioManager pointer used by the GUI system.
 * @param audioMgr  Pointer to the AudioManager instance.
 */

void GuiSystem::SetAudioManager(AudioManager* audioMgr) {
    audio = audioMgr;
}

/**
 * @brief   Helper to apply the current settings volume to the music channel.
 * @param v Normalized volume (0.0–1.0).
 */
static void ApplyNewVolume(float v)
{
    FMOD::Channel* ch = AudioManager::Get().GetCurrentMusicChannel();
    if (ch)
    {
        ch->setVolume(std::clamp(v, 0.f, 1.f));
    }
}

/**
 * @brief   Initializes the GUI system and registers all button actions.
 *          Clears all UI containers, resets ID allocation and registers:
 *          - Scene loading actions (LoadScene, LoadSettings, LoadControls)
 *          - Quit the application
 *          - Volume up/down + save settings
 *          - Controls page paging actions
 */

void GuiSystem::Initialize() {
    buttons.clear();
    textboxes.clear();
    dialogues.clear();
    displays.clear();
    nextUIID = 1; // start fresh

    // Register button actions

    // Go to another scene!
    RegisterAction("LoadScene",
        [](const json& p)
        {
            // Go to another scene!
            if (!p.contains("scene")) { throw std::runtime_error("Button call load scene but params wrong"); }
            std::string scene = FilePath::SAVES_FOLDER + std::string(p.at("scene")) + ".json";
            SceneManager::Get().LoadScene(scene);
            SceneManager::Get().SetTransitionTime(p.at("duration"));
        });

    // Quit the application :(
    RegisterAction("Quit",
        [](const json&)
        {
            CoreEngine::GetInstance().Quit();
        });

    // Load the Settings scene from settings.json, and prime volume from the file
    RegisterAction("LoadSettings",
        [](const json&)
        {
            SceneManager::Get().LoadScene(FilePath::SETTINGS_FILE);
            SceneManager::Get().SetTransitionTime(0.f);

            float newVolume01 = 1.0f;
            try
            {
                std::ifstream ifs(FilePath::SETTINGS_FILE);
                if (ifs)
                {
                    json j;
                    ifs >> j;

                    if (j.contains("settings") && j["settings"].is_object())
                    {
                        const auto& js = j["settings"];
                        if (js.contains("volume"))
                        {
                            float volPct = js["volume"].get<float>();
                            newVolume01 = std::clamp(volPct / 100.0f, 0.0f, 1.0f);
                        }
                    }
                }
            }
            catch (...) {
            }

            settingsVolume = newVolume01;
            if (GuiSystem::audio)
            {
                ApplyNewVolume(settingsVolume);
            }

            settingsNeedSync = true;

            auto& gui = GuiSystem::Get();
            if (settingsNeedSync)
            {
                for (auto& v : gui.Displays())
                {
                    if (v.ID == settingsBarFillID)
                    {
                        v.baseSize.x = settingsBarMaxW * settingsVolume;
                        v.size.x = v.baseSize.x;
                        break;
                    }
                }
            }
        });

    // Decrease volume
    RegisterAction("VolDown",
        [](const json&)
        {
            settingsVolume = std::clamp(settingsVolume - volStep, 0.f, 1.f);
            if (GuiSystem::audio) ApplyNewVolume(settingsVolume);

            auto& gui = GuiSystem::Get();
            for (auto& v : gui.Displays())
                if (v.ID == settingsBarFillID) { v.baseSize.x = settingsBarMaxW * settingsVolume; break; }
        });

    // Increase volume
    RegisterAction("VolUp",
        [](const json&)
        {
            settingsVolume = std::clamp(settingsVolume + volStep, 0.f, 1.f);
            if (GuiSystem::audio) ApplyNewVolume(settingsVolume);

            auto& gui = GuiSystem::Get();
            for (auto& v : gui.Displays())
                if (v.ID == settingsBarFillID) { v.baseSize.x = settingsBarMaxW * settingsVolume; break; }
        });

    // Persist volume back to settings.json
    RegisterAction("SaveSettings",
        [](const json&)
        {
            json j;
            try {
                std::ifstream ifs(FilePath::SETTINGS_FILE);
                if (ifs) ifs >> j;
            }
            catch (...) { /* start fresh on error */ }

            j["settings"]["volume"] = std::round(settingsVolume * 100.0f);

            if (!j.contains("resolutionWidth"))  j["resolutionWidth"] = 1920;
            if (!j.contains("resolutionHeight")) j["resolutionHeight"] = 1080;
            if (!j.contains("fullscreen"))       j["fullscreen"] = true;

            try {
                std::ofstream ofs(FilePath::SETTINGS_FILE, std::ios::trunc);
                ofs << j.dump(4);
            }
            catch (...) {
                std::cerr << "[Settings] Failed to write settings.json\n";
            }
        });

    // Load "How to Play" page / the Controls scene from controls.json
    RegisterAction("LoadControls",
        [](const json&)
        {
            SceneManager::Get().LoadScene(FilePath::CONTROLS_FILE);
            SceneManager::Get().SetTransitionTime(0.f);
        });

    // Controls page 1: show the first page of textboxes
    RegisterAction("ControlsPage1",
        [](const json&)
        {
            auto& gui = GuiSystem::Get();
            auto& tbs = gui.TextBoxes();
            auto& btns = gui.Buttons();

            for (auto& tb : tbs)
            {
                if (tb.ID >= 5 && tb.ID <= 14) tb.isVisible = true;
                else if (tb.ID >= 15) tb.isVisible = false;
            }

            for (auto& b : btns)
            {
                if (b.ID == 2)      b.isVisible = false;
                else if (b.ID == 3) b.isVisible = true;
            }
        });

    // Controls page 1: show the second page of textboxes
    RegisterAction("ControlsPage2",
        [](const json&)
        {
            auto& gui = GuiSystem::Get();
            auto& tbs = gui.TextBoxes();
            auto& btns = gui.Buttons();

            for (auto& tb : tbs)
            {
                if (tb.ID >= 5 && tb.ID <= 14) tb.isVisible = false;
                else if (tb.ID >= 15) tb.isVisible = true;
            }

            for (auto& b : btns)
            {
                if (b.ID == 2)      b.isVisible = true;
                else if (b.ID == 3) b.isVisible = false;
            }
        });
}

/**
 * @brief       Adds a new button to the GUI system with a unique ID.
 * @param cfg   The button configuration to add.
 */

void GuiSystem::AddButton(const ButtonConfig& cfg)
{
    ButtonConfig copy = cfg;
    copy.basePos = cfg.pos;
    copy.baseSize = cfg.size;
    copy.baseTextScale = cfg.textScale;
    copy.onClick = nullptr;
    if (copy.ID <= 0) copy.ID = nextUIID++;
    else nextUIID = std::max(nextUIID, copy.ID + 1);
    BindButtonAction(copy);
    buttons.push_back(copy);
}

/**
 * @brief       Adds a new textbox to the GUI system with a unique ID.
 * @param cfg   The textbox configuration to add.
 */

void GuiSystem::AddTextBox(const TextBoxConfig& cfg)
{
    TextBoxConfig copy = cfg;
    copy.basePos = cfg.pos;
    copy.baseScale = cfg.scale;
    if (copy.ID <= 0) copy.ID = nextUIID++;
    else nextUIID = std::max(nextUIID, copy.ID + 1);
    textboxes.push_back(copy);
}

/**
 * @brief       Adds a new dialogue to the GUI system with a unique ID.
 * @param cfg   The dialogue configuration to add.
 */

void GuiSystem::AddDialogue(const DialogueConfig& cfg)
{
    DialogueConfig copy = cfg;
    if (copy.ID <= 0) copy.ID = nextUIID++;
    else nextUIID = std::max(nextUIID, copy.ID + 1);
    dialogues.push_back(std::move(copy));
}

/**
 * @brief       Adds a new display to the GUI system with a unique ID.
 * @param cfg   The display configuration to add.
 */

void GuiSystem::AddDisplay(const DisplayConfig& cfg)
{
    DisplayConfig copy = cfg;
    copy.basePos = cfg.pos;
    copy.baseSize = cfg.size;
    if (copy.ID <= 0) copy.ID = nextUIID++;
    else nextUIID = std::max(nextUIID, copy.ID + 1);
    displays.push_back(std::move(copy));
}

/**
 * @brief           Helper to spawn a DialogueConfig from a JSON dialogue file and textbox template.
 * @param jsonPath  Path to the dialogue JSON file.
 * @param textBox   Template textbox configuration (position, scale, etc.).
 * @param sceneID   Scene ID within the dialogue file (defaults to "0").
 * @return          Index of the newly added dialogue in dialogues container, or -1 on failure.
 */

int GuiSystem::AddDialogueFromJson(const std::string& jsonPath, const TextBoxConfig& textBox, const std::string& sceneID)
{
    auto owned = std::make_shared<TextSystem::TextComponent>();
    if (!TextSystem::LoadDialogueFromJson(jsonPath, *owned)) return -1;

    DialogueConfig d;
    d.owned = std::move(owned);
    d.textComp = nullptr;
    d.textBox = textBox;
    d.jsonPath = jsonPath;
    d.sceneID = sceneID;

    if (d.ID <= 0) d.ID = nextUIID++;
    else nextUIID = std::max(nextUIID, d.ID + 1);

    dialogues.push_back(std::move(d));
    return static_cast<int>(dialogues.size()) - 1;
}

/**
 * @brief           Enables or disables GUI edit mode.
 * @param isEditing True to enable edit mode, false to disable.
 */

void GuiSystem::SetEditMode(bool isEditing)
{
    editMode = isEditing;
}

/**
 * @brief   Returns whether GUI is currently in edit mode.
 * @return  True if editing, false otherwise.
 */

bool GuiSystem::GetEditMode() const
{
    return editMode;
}

/**
 * @brief       Main per-frame update for the GUI system.
 *              Functions:
 *              - Handles dialogue progression on SPACE (when not editing)
 *              - Converts mouse coords into game-view space
 *              - Builds draw order and render all UI elements (buttons, textboxes, dialogues, displays)
 *              - Applies transition overlay and state window rendering
 * @param dt    Delta time in seconds since last frame.
 */

void GuiSystem::Update(float dt)
{
    SceneManager& sm = SceneManager::Get();
    static bool prev = sm.inEditMode;
    if (prev != sm.inEditMode) 
    {
        for (auto& d : dialogues)
        {
            if (d.isVisible)
            {
                if (auto* comp = (d.textComp ? d.textComp : d.owned.get()))
                {
                    const auto& lines = comp->GetLines();
                    if (lines.empty()) continue;
                    comp->SetCurrentLineIndex(std::clamp(comp->GetCurrentLineIndex(), 0, (int)lines.size() - 1));
                    if (comp->GetTextSpeed() <= 0.f) comp->SetTextSpeed(30.f);
                    comp->StartTyping();
                }
            }
        }
        prev = sm.inEditMode;
    }

    // --- reset typing on editor toggle ---
    static bool prevEdit = false;
    if (prevEdit && !sm.inEditMode) 
    {
        for (auto& d : dialogues) 
        {
            if (!d.isVisible) continue;
            auto* c = d.textComp ? d.textComp : d.owned.get();
            if (!c) continue;
            c->SetVisibleChars(0);
            c->SetTimeElapsed(0.f);
            c->SetIsTyping(true);
        }
    }
    prevEdit = sm.inEditMode;

    // --- SPACE advances dialogue when NOT editing ---
    static bool spaceHandled = false;
    const bool spacePressed = Input::IsKeyPressed(GLFW_KEY_SPACE);
    const bool spaceHeld = Input::IsKeyHeld(GLFW_KEY_SPACE);

    // toggle fps
    if (Input::IsKeyPressed(GLFW_KEY_F3))
    {
        showFPS = !showFPS;
    }

    if (!sm.inEditMode && spacePressed && !spaceHandled)
    {
        for (auto& d : dialogues)
        {
            if (!d.isVisible) continue;
            auto* comp = d.textComp ? d.textComp : d.owned.get();
            if (!comp) continue;

            if (comp->CheckIfTyping())
            {
                comp->StopTyping();
            }
            else
            {
                const auto& lines = comp->GetLines();
                int idx = comp->GetCurrentLineIndex();
                if (idx + 1 < static_cast<int>(lines.size()))
                {
                    comp->SetCurrentLineIndex(idx + 1);
                    comp->SetVisibleChars(0);
                    comp->SetTimeElapsed(0.f);
                    comp->StartTyping();
                }
            }
        }
        spaceHandled = true;
    }
    if (!spaceHeld) spaceHandled = false;

    if (!sm.inEditMode)
    {
        for (auto& d : dialogues) if (d.isVisible)
        {
            if (auto* comp = d.textComp ? d.textComp : d.owned.get())
            {
                TextSystem::UpdateTyping(*comp, dt, 1.f);
            }
        }
    }

    // Mouse (convert to GameView space if ImGui is active)
    Vec2 mouse = Input::GetMousePosition();
    GuiSystem::GetMouseImgui(mouse);

    // Render setup (UI in screen space)
    // Render& render = Render::GetInstance();              // <-- restored local renderer
    const bool prevCam = render.useCamera;
    render.useCamera = false;

    // UI scaling against 1920x1080 virtual space
    const float defaultWidth = 1920.f;
    const float defaultHeight = 1080.f;
    const float scaleX = GLApp::width / defaultWidth;
    const float scaleY = GLApp::height / defaultHeight;
    const float uiScale = std::min(scaleX, scaleY);

    // Build draw order: ALL UI elements sorted by ascending ID
    auto drawList = BuildSortedDrawList();

    // Draw UI elements per type
    
    // Draw button
    // --------------------------------------------------
    auto drawButtonAt = [&](int i)
        {
            auto& cfg = buttons[i];
            if (!cfg.isVisible) return;

            cfg.size = cfg.baseSize * uiScale;
            cfg.textScale = cfg.baseTextScale * uiScale;

            cfg.pos.x = cfg.basePos.x * scaleX;
            cfg.pos.y = cfg.basePos.y * scaleY;

            bool hover = false;
            if (cfg.m_renderData.GetRenderType() == UIRenderType::Shape &&
                cfg.m_renderData.GetShapeData().shape == ButtonShape::Circle)
            {
                float radius = std::min(cfg.size.x, cfg.size.y) * 0.5f;
                float dist = RG_Distance(mouse, cfg.pos);
                hover = dist <= radius;
            }
            else {
                hover = render.IsAreaTouched(mouse, cfg.pos, cfg.size.x, cfg.size.y);
            }

            // New unified renderer
            cfg.DrawButton(hover);

            const float textScale = cfg.textScale;
            Vec2 textSize = render.CalculateTextSize(cfg.label, textScale, "Marker");

            // center text on button
            float textX = cfg.pos.x - textSize.x * 0.5f;
            float textY = cfg.pos.y - textSize.y * 0.5f;
            textY += textSize.y * 0.15f; // small vertical offset

            render.DrawText(cfg.label, textX, textY, textScale, { 1, 1, 1 }, "Marker");

#ifndef NO_IMGUI  //EDITORMACRO
            if (!SceneManager::Get().inEditMode) {
#endif
                if (hover && Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
                    if (audio) audio->PlayButtonSound(cfg.soundType);
                    if (cfg.onClick) cfg.onClick();
                    if (cfg.actionName != "") CallAction(cfg.actionName, cfg.actionParams);
                }
#ifndef NO_IMGUI //EDITORMACRO
            }
#endif
        }; // end DRAW UI ELEMENTS PER TYPE
    // --------------------------------------------------

    // Draw text box
    // --------------------------------------------------
    auto drawTextBoxAt = [&](int i)
        {
            auto& tb = textboxes[i];
            if (!tb.isVisible) return;

            tb.pos.x = tb.basePos.x * scaleX;
            tb.pos.y = tb.basePos.y * scaleY;
            tb.scale = tb.baseScale * uiScale;

            Vec2 textSize = render.CalculateTextSize(tb.text, tb.scale, tb.fontKey);

            float drawX = tb.pos.x;
            float drawY = tb.pos.y;
            if (tb.centered) {
                drawX -= textSize.x * 0.5f;
                drawY -= textSize.y * 0.5f;
            }

            render.DrawText(tb.text, drawX, drawY, tb.scale, tb.color, tb.fontKey);
        }; // end DRAW TEXTBOX
    // --------------------------------------------------

    // Draw dialogue
    // --------------------------------------------------
    auto drawDialogueAt = [&](int i)
        {
            auto& d = dialogues[i];
            if (!d.isVisible) return;

            TextSystem::TextComponent* comp = d.textComp ? d.textComp : d.owned.get();
            if (!comp) return;

            // Skin scale
            d.textBox.pos.x = d.textBox.basePos.x * scaleX;
            d.textBox.pos.y = d.textBox.basePos.y * scaleY;
            d.textBox.scale = d.textBox.baseScale * uiScale;

            // Resolve full line (for size), and visible string (typewriter or full)
            const auto& lines = comp->GetLines();
            if (lines.empty()) return;

            int idx = comp->GetCurrentLineIndex();
            if (idx < 0) idx = 0;
            if (idx >= (int)lines.size()) idx = (int)lines.size() - 1;

            const std::string& fullLine = comp->GetCurrentLine(idx);
            std::string vis;

            if (sm.inEditMode) {
                // In edit mode: show full line, disable typing
                if (comp->CheckIfTyping()) comp->StopTyping();
                vis = lines[idx];
            }
            else {
                // In play mode: typewriter
                // TextSystem::UpdateTyping(*comp, dt, 1.f);
                vis = comp->CheckIfTyping() ? TextSystem::GetVisibleText(*comp) : lines[idx];
            }

            // Get speaker name
            auto getSpeakerName = [&](const json& doc, int sceneIdx, int lineIdx) -> std::string
                {
                    if (!doc.contains("scenes") || !doc["scenes"].is_array()) return {};
                    if (sceneIdx < 0 || sceneIdx >= (int)doc["scenes"].size()) return {};

                    const auto& sc = doc["scenes"][sceneIdx];
                    if (!sc.contains("script") || !sc["script"].is_array()) return {};

                    const auto& script = sc["script"];
                    if (lineIdx < 0 || lineIdx >= (int)script.size()) return {};

                    int speakerID = -1;
                    if (script[lineIdx].contains("speakerID") && script[lineIdx]["speakerID"].is_number_integer())
                        speakerID = script[lineIdx]["speakerID"].get<int>();
                    if (speakerID < 0) return {};

                    if (!sc.contains("speakers") || !sc["speakers"].is_array()) return {};
                    const auto& speakers = sc["speakers"];
                    if (speakerID >= 0 && speakerID < (int)speakers.size() && speakers[speakerID].is_string())
                        return speakers[speakerID].get<std::string>();

                    return {};
                };

            static json sDlgDoc;
            static bool sDlgLoaded = false;
            if (!sDlgLoaded)
            {
                try
                {
                    std::ifstream ifs(FilePath::DIALOGUE);
                    if (ifs) { ifs >> sDlgDoc; sDlgLoaded = true; }
                }
                catch (...) { /* leave unloaded on error */ }
            }

            int sceneIdxInt = 0;
            if (!d.sceneID.empty())
            {
                try { sceneIdxInt = std::stoi(d.sceneID); }
                catch (...) { sceneIdxInt = 0; }
            }

            std::string speakerName;
            if (sDlgLoaded && sDlgDoc.contains("scenes") && sDlgDoc["scenes"].is_array())
            {
                int maxScene = (int)sDlgDoc["scenes"].size() - 1;
                if (maxScene >= 0)
                {
                    sceneIdxInt = std::max(0, std::min(sceneIdxInt, maxScene));
                    speakerName = getSpeakerName(sDlgDoc, sceneIdxInt, idx);
                    if (d.customSpeaker)
                    {
                        speakerName = d.customSpeakerName;
                    }
                }
            }

            // Draw positions
            Colour3 drawColor = d.textBox.color;
            Vec2 fullSize = render.CalculateTextSize(fullLine, d.textBox.scale, "Marker");
            float drawX = d.textBox.pos.x;
            float drawY = d.textBox.pos.y;
            if (d.textBox.centered)
            {
                drawX -= fullSize.x * 0.5f;
                drawY -= fullSize.y * 0.5f;
            }

            // Draw speaker name (no typewriter)
            Vec2 speakerPos{ d.speakerNameBasePos.x * scaleX, d.speakerNameBasePos.y * scaleY };
            float speakerScale = d.speakerNameBaseScale * uiScale;
            Vec2 speakerSize = render.CalculateTextSize(speakerName, speakerScale, "Marker");
            float speakerX = speakerPos.x, speakerY = speakerPos.y;
            if (d.speakerNameCentered) 
            { 
                speakerX -= speakerSize.x * 0.5f; 
                speakerY -= speakerSize.y * 0.5f; 
            }
            render.DrawText(speakerName, speakerX, speakerY, speakerScale, d.speakerNameColor, "Marker");

            //const float nameScale = d.textBox.scale * 0.9f;
            const float lineH = render.CalculateTextSize("A", d.textBox.scale, "Marker").y;
            //render.DrawText(speakerName, drawX, drawY + (lineH * 1.5f), nameScale, drawColor, "Marker");

            // Word-wrap visible text
            const float maxWidth = ((d.wrapWidth > 0.f) ? d.wrapWidth : (defaultWidth / 2.f)) * uiScale;
            std::vector<std::string> wrappedLines;
            wrappedLines.reserve(8);

            std::string current;
            size_t iChar = 0;
            while (iChar < vis.size())
            {
                while (iChar < vis.size() && vis[iChar] == ' ')
                    ++iChar;

                size_t start = iChar;
                while (iChar < vis.size() && vis[iChar] != ' ')
                    ++iChar;

                std::string word = vis.substr(start, iChar - start);
                if (word.empty())
                {
                    if (iChar < vis.size() && vis[iChar] == ' ') ++iChar;
                    continue;
                }

                std::string trial = current.empty() ? word : (current + " " + word);
                if (render.CalculateTextSize(trial, d.textBox.scale, "Marker").x <= maxWidth)
                {
                    current = std::move(trial);
                }
                else {
                    if (!current.empty())
                    {
                        wrappedLines.push_back(current);
                        current.clear();
                    }

                    if (render.CalculateTextSize(word, d.textBox.scale, "Marker").x > maxWidth)
                    {
                        std::string chunk;
                        for (char c : word)
                        {
                            std::string attempt = chunk; attempt.push_back(c);
                            if (render.CalculateTextSize(attempt, d.textBox.scale, "Marker").x <= maxWidth)
                            {
                                chunk.push_back(c);
                            }
                            else
                            {
                                if (!chunk.empty()) wrappedLines.push_back(chunk);
                                chunk.clear();
                                chunk.push_back(c);
                            }
                        }
                        current = chunk;
                    }
                    else
                    {
                        current = word;
                    }
                }
                if (iChar < vis.size() && vis[iChar] == ' ') ++iChar;
            }
            if (!current.empty()) wrappedLines.push_back(current);

            // Render wrapped lines
            float yCursor = drawY;
            for (const auto& ln : wrappedLines)
            {
                render.DrawText(ln, drawX, yCursor, d.textBox.scale, drawColor, "Marker");
                yCursor -= lineH;
            }
        }; // end DRAW DIALOGUE
        // --------------------------------------------------

    // Draw display
    // --------------------------------------------------
    auto drawDisplayAt = [&](int i)
    {
        auto& dp = displays[i];
        if (!dp.isVisible) return;

        dp.size = dp.baseSize * uiScale;
        dp.pos.x = dp.basePos.x * scaleX;
        dp.pos.y = dp.basePos.y * scaleY;

        switch (dp.m_renderData.GetRenderType())
        {
        case UIRenderType::Shape:
        {
            UIShapeRender data = dp.m_renderData.GetShapeData();
            switch (data.shape)
            {
            case ButtonShape::Rectangle:
            {
                render.DrawRect(dp.pos, dp.size.x, dp.size.y, data.color);
                break;
            }
            case ButtonShape::Circle:
            {
                render.DrawCircle(dp.pos, std::min(dp.size.x, dp.size.y) * 0.5f, data.color, 48);
                break;
            }
            case ButtonShape::Rounded:
            {
                render.DrawRoundedRect(dp.pos, dp.size.x, dp.size.y, std::min(dp.size.x, dp.size.y) * 0.5f, data.color);
                break;
            }
            } break;
        }
        case UIRenderType::CustomQuad:
        {
            auto data = dp.m_renderData.GetQuadData();

            float sx = GLApp::width / 1920.f;
            float sy = GLApp::height / 1080.f;

            auto scale = [&](Vec2 v) { return Vec2{ v.x * sx, v.y * sy };}; //lambda function to calculate scale value for full screen

            render.DrawCustomQuad(scale(data.v0), scale(data.v1), scale(data.v2),scale(data.v3),data.color); //then use scaled values

            //render.DrawCustomQuad(data.v0, data.v1, data.v2, data.v3, data.color);
        } break;

        case UIRenderType::Image:
        {
            auto data = dp.m_renderData.GetImageData();
            render.DrawImageRotated(data.m_image, dp.pos.x, dp.pos.y, dp.size.x, dp.size.y, 0.f, 0.f);
        } break;

        case UIRenderType::ImageSheet:
        {
            auto data = dp.m_renderData.GetImageSheetData();
            int tileW = data.m_image.widthPixels / data.total_cols;
            int tileH = data.m_image.heightPixels / data.total_rows;
            int col = data.frame_index.curr_col;
            int row = data.total_rows - 1 - data.frame_index.curr_row;
            float u0 = float(col * tileW), v0 = float(row * tileH);
            float u1 = u0 + tileW, v1 = v0 + tileH;

            render.DrawImageSub(data.m_image, dp.pos.x, dp.pos.y, dp.size.x, dp.size.y, u0, v0, u1, v1, 0.f, 0.f);
        } break;

        default: break;
        }
    }; // end DRAW DISPLAY
    // --------------------------------------------------

    // Sync for settings volume bar after scene load
    if (settingsNeedSync)
    {
        for (auto& dp : displays)
        {
            if (dp.ID == settingsBarFillID)
            {
                dp.baseSize.x = settingsBarMaxW * settingsVolume;
                dp.size.x = dp.baseSize.x;
                settingsNeedSync = false;
                break;
            }
        }
    }

    // Draw UI elements according to ascending ID
    for (const auto& it : drawList)
    {
        switch (it.type)
        {
        case SelectedType::Button:
            if (it.index >= 0 && it.index < (int)buttons.size())   drawButtonAt(it.index);
            break;
        case SelectedType::TextBox:
            if (it.index >= 0 && it.index < (int)textboxes.size()) drawTextBoxAt(it.index);
            break;
        case SelectedType::Dialogue:
            if (it.index >= 0 && it.index < (int)dialogues.size()) drawDialogueAt(it.index);
            break;
        case SelectedType::Display:
            if (it.index >= 0 && it.index < (int)displays.size()) drawDisplayAt(it.index);
            break;
        default: break;
        }
    }

    // --- State window over UI ---
    if (stateWindowActive)
    {
        DrawStateWindow(render);
    }

#ifndef NO_IMGUI
    if (!SceneManager::Get().inEditMode)
    {
#endif

    static bool opencontrol = false;
    static int controlpage = 0;
    // Pause menu
    if (SceneManager::Get().IsPaused())
    {
        // const float defaultW = 1920.f;
        // const float defaultH = 1080.f;
        // const float scaleX = GLApp::width / defaultW;
        // const float scaleY = GLApp::height / defaultH;
        // const float uiScale = std::min(scaleX, scaleY);
        // Title text (scaled like UI text)
        float baseTextScale = 1.0f;
        const float textScale = baseTextScale * uiScale;
        Render& renderer = Render::GetInstance();
        if (!opencontrol)
        {


        renderer.DrawRect({ 960.f * scaleX, 540.f * scaleY }, 3000.f * scaleX, 2000.f * scaleY, { 0.f,0.f,0.f }, 0.f, 0.5f);

        Image img = CreateImage("../Assets/UI_ExitPanel.PNG");
        renderer.DrawImageRotated(img, 960 * scaleX, 540 * scaleY, 1000 * scaleX, 1000 * scaleY);
        bool hovercontrol = false;
        bool hoverexit = false;
        bool hoverresume = false;

        {
            hovercontrol = (renderer.IsAreaTouched(mouse, { 960 * scaleX,400 * scaleY }, 400 * scaleX, 190 * scaleY));
            float brightness = hovercontrol ? 0.2f : 0.f;
            Image btnimg = CreateImage("../Assets/UI_MainMenu_NewGame_Button.png");
            renderer.DrawImageRotated(btnimg, 960.f * scaleX, 400.f * scaleY, 400.f * scaleX, 190.f * scaleY, 0.f, brightness);

            const Vec2 baseTitlePos = { 960.f * scaleX, 400.f * scaleY};
            const std::string title = "how to play";
            Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
            float textX = baseTitlePos.x - textSize.x * 0.5f;
            float textY = baseTitlePos.y - textSize.y * 0.5f + textSize.y * 0.15f;
            render.DrawText(title, textX, textY, textScale, { 1,1,1 }, "Marker");

        }

        {
            hoverexit = (renderer.IsAreaTouched(mouse, { 960 * scaleX,600 * scaleY}, 400 * scaleX, 190 * scaleY));
            float brightness = hoverexit ? 0.2f : 0.f;
            Image btnimg = CreateImage("../Assets/UI_MainMenu_QuitButton.png");
            renderer.DrawImageRotated(btnimg, 960.f * scaleX, 600.f * scaleY, 400.f * scaleX, 190.f * scaleY, 0.f, brightness);

            const Vec2 baseTitlePos = { 960.f * scaleX, 600.f * scaleY };
            const std::string title = "QUIT GAME";
            Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
            float textX = baseTitlePos.x - textSize.x * 0.5f;
            float textY = baseTitlePos.y - textSize.y * 0.5f + textSize.y * 0.15f;
            render.DrawText(title, textX, textY, textScale, { 1,1,1 }, "Marker");

        }


        {
            hoverresume = (renderer.IsAreaTouched(mouse, { 960 * scaleX,800 * scaleY }, 400 * scaleX, 190 * scaleY));
            float brightness = hoverresume ? 0.2f : 0.f;
            Image btnimg = CreateImage("../Assets/UI_MainMenu_NewGame_Button.png");
            renderer.DrawImageRotated(btnimg, 960.f * scaleX, 800.f * scaleY, 400.f * scaleX, 190.f * scaleY, 0.f, brightness);

            const Vec2 baseTitlePos = { 960.f * scaleX, 800.f * scaleY };
            const std::string title = "RESUME GAME";
            Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
            float textX = baseTitlePos.x - textSize.x * 0.5f;
            float textY = baseTitlePos.y - textSize.y * 0.5f + textSize.y * 0.15f;
            render.DrawText(title, textX, textY, textScale, { 1,1,1 }, "Marker");

        }



        if (hovercontrol && Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            opencontrol = true;
            controlpage = 0;
        }

        if (hoverexit && Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            SceneManager::Get().QueueLoadScene("../Assets/saves/MainMenu.json");
            SceneManager::Get().SetTransitionTime(2.f);
        }

        if (hoverresume && Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            SceneManager::Get().TogglePause();
        }

        }
        else
        {
            Image pagebg = CreateImage("../Assets/UI_ControlsPage_Background.PNG");
            renderer.DrawImageRotated(pagebg, 960.f * scaleX, 540.f * scaleY, 1920.f * scaleX, 1080.f * scaleY);
            baseTextScale = 0.8f;


            bool hoverleave = (renderer.IsAreaTouched(mouse, { 100 * scaleX,1000 * scaleY }, 200 * scaleX, 180 * scaleY));
            {
                Image exitbtn = CreateImage("../Assets/UI_ControlsPage_BackButton.png");
                float brightness = hoverleave ? 0.2f : 0.f;
                renderer.DrawImageRotated(exitbtn, 100.f * scaleX, 1000.f * scaleY, 200.f * scaleX, 180.f * scaleY, 0.f, brightness);
            }
            if (hoverleave && Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
            {
                controlpage = 0;
                opencontrol = false;
            }

            if (controlpage == 0)
            {
                {
                    const Vec2 baseTitlePos = { 960.f * scaleX, 900.f * scaleY };
                    const std::string title = "CONTROLS";
                    Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
                    // float textX = baseTitlePos.x - textSize.x * .8f;
                    // float textY = baseTitlePos.y - textSize.y * .8f + textSize.y * 0.15f;
                    render.DrawText(title, baseTitlePos.x, baseTitlePos.y, textScale, { 1,1,1 }, "Marker");
                }

                {
                    const Vec2 baseTitlePos = { 200.f * scaleX, 650.f * scaleY };
                    const std::string title = "W - Move Up";
                    Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
                    render.DrawText(title, baseTitlePos.x, baseTitlePos.y, textScale, { 1,1,1 }, "Marker");
                }

                {
                    const Vec2 baseTitlePos = { 200.f * scaleX, 600.f * scaleY };
                    const std::string title = "S - Move Down";
                    Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
                    render.DrawText(title, baseTitlePos.x, baseTitlePos.y, textScale, { 1,1,1 }, "Marker");
                }

                {
                    const Vec2 baseTitlePos = { 200.f * scaleX, 550.f * scaleY };
                    const std::string title = "A - Move Left";
                    Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
                    render.DrawText(title, baseTitlePos.x, baseTitlePos.y, textScale, { 1,1,1 }, "Marker");
                }

                {
                    const Vec2 baseTitlePos = { 200.f * scaleX, 500.f * scaleY };
                    const std::string title = "D - Move Right";
                    Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
                    render.DrawText(title, baseTitlePos.x, baseTitlePos.y, textScale, { 1,1,1 }, "Marker");
                }

                {
                    const Vec2 baseTitlePos = { 200.f * scaleX, 450.f * scaleY };
                    const std::string title = "Shift - Dash/Dodge";
                    Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
                    render.DrawText(title, baseTitlePos.x, baseTitlePos.y, textScale, { 1,1,1 }, "Marker");
                }

                {
                    const Vec2 baseTitlePos = { 800.f * scaleX, 650.f * scaleY };
                    const std::string title = "Arrow Keys - Spell Input Sequence";
                    Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
                    render.DrawText(title, baseTitlePos.x, baseTitlePos.y, textScale, { 1,1,1 }, "Marker");
                }

                {
                    const Vec2 baseTitlePos = { 800.f * scaleX, 600.f * scaleY };
                    const std::string title = "1 / 2 / 3 - Use Song Slot 1-2-3";
                    Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
                    render.DrawText(title, baseTitlePos.x, baseTitlePos.y, textScale, { 1,1,1 }, "Marker");
                }

                {
                    const Vec2 baseTitlePos = { 800.f * scaleX, 550.f * scaleY };
                    const std::string title = "Q - Cycle Through Element";
                    Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
                    render.DrawText(title, baseTitlePos.x, baseTitlePos.y, textScale, { 1,1,1 }, "Marker");
                }

                {
                    const Vec2 baseTitlePos = { 800.f * scaleX, 500.f * scaleY };
                    const std::string title = "E - Melee Attack";
                    Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
                    render.DrawText(title, baseTitlePos.x, baseTitlePos.y, textScale, { 1,1,1 }, "Marker");
                }

                {
                    const Vec2 baseTitlePos = { 800.f * scaleX, 450.f * scaleY };
                    const std::string title = "ESC - Pause/Resume Game";
                    Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");

                    render.DrawText(title, baseTitlePos.x, baseTitlePos.y, textScale, { 1,1,1 }, "Marker");
                }

                bool hoverpage = (renderer.IsAreaTouched(mouse, { 120 * scaleX,150 * scaleY }, 180 * scaleX, 150 * scaleY));
                {
                    Image pgbtn = CreateImage("../Assets/UI_ControlsPage_ForwardPageLogo.png");
                    float brightness = hoverpage ? 0.2f : 0.f;
                    renderer.DrawImageRotated(pgbtn, 120.f * scaleX, 100 * scaleY, 180.f * scaleX, 150.f * scaleY, 0.f, brightness);
                }
                if (hoverpage && Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
                {
                    controlpage = 1;
                }
            }
            else 
            {
                {
                    const Vec2 baseTitlePos = { 700.f * scaleX, 650.f * scaleY };
                    const std::string title = "Song List:";
                    Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
                    render.DrawText(title, baseTitlePos.x, baseTitlePos.y, textScale, { 1,1,1 }, "Marker");
                }

                {
                    const Vec2 baseTitlePos = { 700.f * scaleX, 600.f * scaleY };
                    const std::string title = "Soundwave Attack -> Up, Down";
                    Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
                    render.DrawText(title, baseTitlePos.x, baseTitlePos.y, textScale, { 1,1,1 }, "Marker");
                }

                {
                    const Vec2 baseTitlePos = { 700.f * scaleX, 550.f * scaleY };
                    const std::string title = "AOE Attack -> Up, Left, Down";
                    Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
                    render.DrawText(title, baseTitlePos.x, baseTitlePos.y, textScale, { 1,1,1 }, "Marker");
                }

                {
                    const Vec2 baseTitlePos = { 700.f * scaleX, 500.f * scaleY };
                    const std::string title = "Shield -> Down, Down, Right";
                    Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
                    render.DrawText(title, baseTitlePos.x, baseTitlePos.y, textScale, { 1,1,1 }, "Marker");
                }

                {
                    const Vec2 baseTitlePos = { 700.f * scaleX, 450.f * scaleY };
                    const std::string title = "Healing -> Right, Up, Right";
                    Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
                    render.DrawText(title, baseTitlePos.x, baseTitlePos.y, textScale, { 1,1,1 }, "Marker");
                }

                bool hoverpage = (renderer.IsAreaTouched(mouse, { 120 * scaleX,200 * scaleY }, 180 * scaleX, 150 * scaleY));
                {
                    Image pgbtn = CreateImage("../Assets/UI_ControlsPage_BackwardsPageLogo.png");
                    float brightness = hoverpage ? 0.2f : 0.f;
                    renderer.DrawImageRotated(pgbtn, 120.f * scaleX, 200.f * scaleY, 180.f * scaleX, 150.f * scaleY, 0.f, brightness);
                }
                if (hoverpage && Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
                {
                    controlpage = 0;
                }
            }
        }


    }

#ifndef NO_IMGUI
    }
#endif

    if (showFPS)
    {
        double framesdisplay = CoreEngine::GetInstance().GetFPS();
        // const float defaultW = 1920.f;
        // const float defaultH = 1080.f;
        // const float scaleX = GLApp::width / defaultW;
        // const float scaleY = GLApp::height / defaultH;
        // float uiScale = std::min(scaleX, scaleY);
        // Title text (scaled like UI text)
        float baseTextScale = 1.0f;
        const float textScale = baseTextScale * uiScale;
        render.DrawText("FPS: " + std::to_string(framesdisplay), 50.f, 50.f, textScale, { 1,1,1 }, "Marker");
    }

    // Transition overlay
    {
        float t = SceneManager::Get().GetTransitionProgress();
        if (t != 1.f)
        {
            Vec2 pos = { GLApp::width * 0.5f, GLApp::height * 0.5f };
            render.DrawRect(pos, (float)GLApp::width, (float)GLApp::height, { 0.f,0.f,0.f }, 0.0f, t);
        }
    }
    // restore camera back to world
    render.useCamera = prevCam;
}

/**
 * @brief   Shuts down the GUI system and clears all managed UI elements. 
            Resets the next UI ID to 1.
 */

void GuiSystem::Shutdown() {
    buttons.clear();
    textboxes.clear();
    dialogues.clear();
    displays.clear();
    nextUIID = 1;
    //audio = nullptr;
}

/**
 * @brief       Converts a ButtonShape enum to its corresponding string representation.
 * @param shape ButtonShape enum value.
 * @return      String representation.
 */

std::string ButtonShapeToString(ButtonShape shape) {
    switch (shape) {
    case ButtonShape::Rounded:      return "Rounded";
    case ButtonShape::Rectangle:    return "Rectangle";
    case ButtonShape::Circle:       return "Circle";
    default:                        return "Rectangle";
    }
}

/**
 * @brief   Converts a string into a ButtonShape enum.
 *          Defaults to ButtonShape::Rectangle if the string is unrecognized.
 * @param s Input string (e.g. "Rounded", "Circle").
 * @return  ButtonShape parsed enum.
 */

ButtonShape StringToButtonShape(const std::string& s) 
{
    if (s == "Rounded") return ButtonShape::Rounded;
    if (s == "Circle")  return ButtonShape::Circle;
    return ButtonShape::Rectangle;
}

/**
 * @brief           Converts a ButtonAction enum to a string.
 * @param action    ButtonAction value.
 * @return          String representation.
 */

std::string ButtonActionToString(ButtonAction action)
{
    switch (action)
    {
    case ButtonAction::None:                return "None";
    case ButtonAction::LoadLevel1:          return "LoadLevel1";
    case ButtonAction::LoadLevel2:          return "LoadLevel2";
    case ButtonAction::LoadMainMenu:        return "LoadMainMenu";
    case ButtonAction::Quit:                return "Quit";
    case ButtonAction::OpenStateWindow:     return "OpenStateWindow";
    case ButtonAction::ToggleAllButtons:    return "ToggleAllButtons";
    case ButtonAction::ToggleAllTextboxes:  return "ToggleAllTextboxes";
    case ButtonAction::ToggleAllDialogues:  return "ToggleAllDialogues";
    case ButtonAction::ToggleAllDisplays:   return "ToggleAllDisplays";
    case ButtonAction::LoadSettings:        return "LoadSettings";
    case ButtonAction::LoadControls:        return "LoadControls";
    default:                                return "None";
    }
}

/**
 * @brief   Converts string representation of action into ButtonAction enum.
 * @param s String (e.g. "LoadMainMenu", "Quit").
 * @return  ButtonAction parsed enum or ButtonAction::None for unknown strings.
 */

ButtonAction StringToButtonAction(const std::string& s)
{
    if (s == "LoadLevel1")          return ButtonAction::LoadLevel1;
    if (s == "LoadLevel2")          return ButtonAction::LoadLevel2;
    if (s == "LoadMainMenu")        return ButtonAction::LoadMainMenu;
    if (s == "Quit")                return ButtonAction::Quit;
    if (s == "OpenStateWindow")     return ButtonAction::OpenStateWindow;
    if (s == "ToggleAllButtons")    return ButtonAction::ToggleAllButtons;
    if (s == "ToggleAllTextboxes")  return ButtonAction::ToggleAllTextboxes;
    if (s == "ToggleAllDialogues")  return ButtonAction::ToggleAllDialogues;
    if (s == "ToggleAllDisplays")   return ButtonAction::ToggleAllDisplays;
    if (s == "LoadSettings")        return ButtonAction::LoadSettings;
    if (s == "LoadControls")        return ButtonAction::LoadControls;
    return ButtonAction::None;
}

/**
 * @brief   Clears all managed UI elements. Resets the next UI ID to 1.
 */

void GuiSystem::Clear()
{
    buttons.clear();
    textboxes.clear();
    dialogues.clear();
    displays.clear();
    m_groups.clear();
    nextUIID = 1;
}

/**
 * @brief       Binds built-in click behaviors to a button based on its ButtonAction.
 * @param btn   The button configuration to modify.
 */

void GuiSystem::BindButtonAction(ButtonConfig& btn)
{
    switch (btn.action)
    {
    case ButtonAction::OpenStateWindow:
        btn.onClick = [this]() { stateWindowActive = true; };
        break;
    case ButtonAction::ToggleAllButtons:
        btn.onClick = [this, &btn]() { ToggleAllButtons(&btn); };
        break;
    case ButtonAction::ToggleAllTextboxes:
        btn.onClick = [this]() { ToggleAllTextboxes(); };
        break;
    case ButtonAction::ToggleAllDialogues:
        btn.onClick = [this]() { ToggleAllDialogues(); };
        break;
    case ButtonAction::ToggleAllDisplays:
        btn.onClick = [this]() { ToggleAllDisplays(); };
        break;
    case ButtonAction::LoadLevel1:
        btn.onClick = [this]() {
            SceneManager::Get().QueueLoadScene("../Assets/saves/level1.json");
            SceneManager::Get().SetTransitionTime(2.f);
            };
        break;
    case ButtonAction::LoadMainMenu:
        btn.onClick = [this]() {
            SceneManager::Get().QueueLoadScene("../Assets/saves/MainMenu.json");
            SceneManager::Get().SetTransitionTime(0.f);
            };
        break;
    case ButtonAction::Quit:
        btn.onClick = [this]() { CoreEngine::GetInstance().Quit(); };
        break;
    case ButtonAction::LoadSettings:
        btn.onClick = [this]() {
            SceneManager::Get().QueueLoadScene(FilePath::SETTINGS_FILE);
            SceneManager::Get().SetTransitionTime(0.f);
            };
        break;
    case ButtonAction::LoadControls:
        btn.onClick = [this]() {
            SceneManager::Get().QueueLoadScene(FilePath::CONTROLS_FILE);
            SceneManager::Get().SetTransitionTime(0.f);
            };
        break;
    default:
        btn.onClick = nullptr;
        break;
    }
}

/**
 * @brief           Toggles visibility of all buttons except an optional excluded one.
 *                  If all considered buttons are visible, hides them; otherwise shows them.
 * @param exclude   Optional button pointer to ignore when toggling.
 */

void GuiSystem::ToggleAllButtons(const ButtonConfig* exclude)
{
    auto& btns = Buttons();
    if (btns.empty()) return;
    bool allVisible = true;
    bool anyConsidered = false;

    for (const auto& b : btns)
    {
        if (&b == exclude) continue;
        anyConsidered = true;
        if (!b.isVisible) { allVisible = false; break; }
    }

    if (!anyConsidered) return;

    const bool newState = !allVisible;

    for (auto& b : btns) {
        if (&b == exclude) continue;
        b.isVisible = newState;
    }
}

/**
 * @brief Toggles visibility of all textboxes at once.
 */

void GuiSystem::ToggleAllTextboxes()
{
    if (textboxes.empty()) return;
    bool anyVisible = false;
    for (auto& t : textboxes) if (t.isVisible) { anyVisible = true; break; }
    for (auto& t : textboxes) t.isVisible = !anyVisible;
}

/**
 * @brief Toggles visibility of all dialogues at once.
 */

void GuiSystem::ToggleAllDialogues()
{
    if (dialogues.empty()) return;
    bool anyVisible = false;
    for (auto& d : dialogues) if (d.isVisible) { anyVisible = true; break; }
    for (auto& d : dialogues) d.isVisible = !anyVisible;
}

/**
 * @brief Toggles visibility of all displays at once.
 */

void GuiSystem::ToggleAllDisplays()
{
    if (displays.empty()) return;
    bool anyVisible = false;
    for (auto& dp : displays) if (dp.isVisible) { anyVisible = true; break; }
    for (auto& dp : displays) dp.isVisible = !anyVisible;
}

/**
 * @brief           Draws the state window overlay UI (e.g. "Choose Your State" menu).
 *                  This overlay is drawn in screen space, including its own buttons and close control.
 * @param render    Reference to the Render instance to use.
 */

void GuiSystem::DrawStateWindow(Render& renderer)
{
    if (!stateWindowActive) return;

    const bool prevCam = renderer.useCamera;
    renderer.useCamera = false;

    const float defaultW = 1920.f;
    const float defaultH = 1080.f;
    const float scaleX = GLApp::width / defaultW;
    const float scaleY = GLApp::height / defaultH;
    const float uiScale = std::min(scaleX, scaleY);

    Vec2 mouse = Input::GetMousePosition();
    GuiSystem::GetMouseImgui(mouse);

    // -------- Window background (scaled like UI) --------
    {
        const Vec2 basePos = { 960.f, 560.f };   // virtual center
        const Vec2 baseSize = { 700.f, 550.f };   // virtual size
        Vec2 pos;   pos.x = basePos.x * scaleX; pos.y = basePos.y * scaleY;
        Vec2 size;  size.x = baseSize.x * uiScale; size.y = baseSize.y * uiScale;
        render.DrawRect(pos, size.x, size.y, { 0.1f, 0.1f, 0.1f });

        // Title text (scaled like UI text)
        const float baseTextScale = 1.0f;
        const float textScale = baseTextScale * uiScale;

        const Vec2 baseTitlePos = { 960.f, 780.f };
        const std::string title = "CHOOSE YOUR STATE";
        Vec2 textSize = render.CalculateTextSize(title, textScale, "Marker");
        float textX = baseTitlePos.x - textSize.x * 0.5f;
        float textY = baseTitlePos.y - textSize.y * 0.5f + textSize.y * 0.15f;
        render.DrawText(title, textX, textY, textScale, { 1,1,1 }, "Marker");
    }

    struct Item { const char* label; ButtonAction action; };
    const Item items[] = {
        {"None",            ButtonAction::None},
        {"Main Menu",       ButtonAction::LoadMainMenu},
        {"Level 1",         ButtonAction::LoadLevel1},
        {"Quit",            ButtonAction::Quit},
        {"Toggle Buttons",  ButtonAction::ToggleAllButtons}
    };

    auto ClickFunction = [this](ButtonAction action)
        {
            ButtonConfig tmp;
            tmp.action = action;
            BindButtonAction(tmp);
            if (tmp.onClick) tmp.onClick();
        };

    // State Window Buttons
    float yVirtual = 720.f;

    for (const auto& it : items)
    {
        if (it.action == ButtonAction::None) { yVirtual -= 40.f; continue; }

        const Vec2 basePos = { 960.f, yVirtual };
        const Vec2 baseSize = { 500.f,  60.f };

        Vec2 pos;  pos.x = basePos.x * scaleX; pos.y = basePos.y * scaleY;
        Vec2 size; size.x = baseSize.x * uiScale; size.y = baseSize.y * uiScale;

        const bool hover = render.IsAreaTouched(mouse, pos, size.x, size.y);

        Colour3 col = hover ? Colour3{ 0.45f, 0.45f, 0.45f } : Colour3{ 0.25f, 0.25f, 0.25f };
        render.DrawRect(pos, size.x, size.y, col);

        const float baseTextScale = 0.7f;
        const float textScale = baseTextScale * uiScale;
        Vec2 textSize = render.CalculateTextSize(it.label, textScale, "Marker");

        float textX = pos.x - textSize.x * 0.5f;
        float textY = pos.y - textSize.y * 0.5f + textSize.y * 0.15f;
        render.DrawText(it.label, textX, textY, textScale, { 1,1,1 }, "Marker");

        // Click action
        if (hover && Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            ClickFunction(it.action);
            render.useCamera = prevCam;
            return;
        }

        yVirtual -= 80.f;
    }

    // Button to close state window
    {
        const Vec2 basePos = { 960.f, yVirtual };
        const Vec2 baseSize = { 200.f,  50.f };

        Vec2 pos;  pos.x = basePos.x * scaleX; pos.y = basePos.y * scaleY;
        Vec2 size; size.x = baseSize.x * uiScale; size.y = baseSize.y * uiScale;

        const bool hover = render.IsAreaTouched(mouse, pos, size.x, size.y);

        render.DrawRect(pos, size.x, size.y, { 0.7f, 0.f, 0.f });

        const char* label = "Close";
        const float baseTextScale = 0.7f;
        const float textScale = baseTextScale * uiScale;
        Vec2 textSize = render.CalculateTextSize(label, textScale, "Marker");
        float textX = pos.x - textSize.x * 0.5f;
        float textY = pos.y - textSize.y * 0.5f + textSize.y * 0.15f;
        render.DrawText(label, textX, textY, textScale, { 1,1,1 }, "Marker");

        if (hover && Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            stateWindowActive = false;
        }
    }

    render.useCamera = prevCam;
}

/**
 * @brief       Converts the given mouse position to game-view coordinates when ImGui is active.
 *              If mouse is outside the game view rect, moves it off-screen so UI hit tests ignore it.
 * @param mouse Reference to mouse position to modify in-place.
 */

void GuiSystem::GetMouseImgui(Vec2& mouse)
{
#ifndef NO_IMGUI
    if (!ImGuiSystem::guiEnabled) 
    {
        // Fullscreen mouse coords are already correct
    }
    else 
    {
        if (mouse.x < m_gameViewRect.x || mouse.x > m_gameViewRect.x + m_gameViewRect.width ||
            mouse.y < m_gameViewRect.y || mouse.y > m_gameViewRect.y + m_gameViewRect.height)
        {
            mouse.x = -9999.f;
            mouse.y = -9999.f;
        }
        else 
        {
            mouse.x -= m_gameViewRect.x;
            mouse.y -= m_gameViewRect.y;

            float mouseScaleX = GLApp::width / m_gameViewRect.width;
            float mouseScaleY = GLApp::height / m_gameViewRect.height;

            mouse.x *= mouseScaleX;
            mouse.y *= mouseScaleY;
        }
    }
#endif
}

/**
 * @brief               Sets the current line of a dialogue text component, optionally restarting typing.
 *                      Bounds-checks the requested index, stops typing if currently typing, and sets
 *                      the new current line index. Optionally starts typing again from the new line.
 * @param dlg           Dialogue configuration whose text component should be updated.
 * @param newIndex      Desired line index within the dialogue lines.
 * @param restartTyping True to restart the typewriter effect; false to leave it stopped.
 */

void GuiSystem::SetDialogueLine(DialogueConfig& dlg, int newIndex, bool restartTyping)
{
    TextSystem::TextComponent* comp = dlg.textComp ? dlg.textComp : dlg.owned.get();
    if (!comp) return;

    const auto& lines = comp->GetLines();
    if (lines.empty()) return;

    if (newIndex < 0) newIndex = 0;
    if (newIndex >= (int)lines.size()) newIndex = (int)lines.size() - 1;
    if (comp->CheckIfTyping()) comp->StopTyping();
    comp->SetCurrentLineIndex(newIndex);
    if (restartTyping) comp->StartTyping();
}

/**
 * @brief   Builds a single draw list of all UI elements sorted by ascending ID.
 *          The resulting DrawItem vector is used in Update() to render UI in a stable order
 *          across all UI types: Buttons, TextBoxes, Dialogues and Displays.
 * @return  Sorted list of drawable UI references.
 */

std::vector<GuiSystem::DrawItem> GuiSystem::BuildSortedDrawList() const
{
    std::vector<DrawItem> out;
    out.reserve(buttons.size() + textboxes.size() + dialogues.size() + displays.size());

    for (int i = 0; i < (int)buttons.size(); ++i) out.push_back({ SelectedType::Button,   i, buttons[i].ID });
    for (int i = 0; i < (int)textboxes.size(); ++i) out.push_back({ SelectedType::TextBox,  i, textboxes[i].ID });
    for (int i = 0; i < (int)dialogues.size(); ++i) out.push_back({ SelectedType::Dialogue, i, dialogues[i].ID });
    for (int i = 0; i < (int)displays.size(); ++i) out.push_back({ SelectedType::Display, i, displays[i].ID });

    std::sort(out.begin(), out.end(), [](const DrawItem& a, const DrawItem& b) {
        if (a.ID != b.ID)   return a.ID < b.ID;                 // Ascending by ID
        if (a.type != b.type) return (int)a.type < (int)b.type;   // Stable across types
        return a.index < b.index;
    });

    return out;
}

/**
 * @brief       Registers an action callback with a string name.
 * @param name  Name of the action.
 * @param func  Callback accepting const json& parameters.
 */

void GuiSystem::RegisterAction(const std::string& name, ActionFunction func) {
    if (actionRegistry.contains(name)) { std::cerr << "Button function " << name << " already registered...\n"; }
    actionRegistry[name] = func;
}

/**
 * @brief           Invokes a previously registered action by name.
 *                  Logs a warning to stderr if the action is not found or throws an exception.
 * @param name      Name of the action to invoke.
 * @param params    JSON parameters to pass to the registered callback.
 * @return true     If the action was found and invoked (even if it threw).
 * @return false    If the action name was not found.
 */

bool GuiSystem::CallAction(const std::string& name, const nlohmann::json& params) {
    auto it = actionRegistry.find(name);

    if (it == actionRegistry.end()) 
    {
        std::cerr << "[GuiSystem] Button action '" << name << "' called but not registered!\n";
        return false;
    }

    try 
    {
        it->second(params);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[GuiSystem] Error calling button action '" << name << "' : " << e.what() << '\n';
    }
    catch (...)
    {
        std::cerr << "[GuiSystem] Error calling button action '" << name << "' but error unknown :(\n";
    }

    return true;
}

// For UI editor
/**
 * @brief   Reseeds the next UI element ID from the maximum existing ID plus one.
 *          Scans all buttons, textboxes, dialogues and displays.
 *          Used when loading scenes to keep IDs globally unique.
 */
void GuiSystem::ReseedNextUIID()
{
    int maxID = 0;
    for (const auto& b : buttons)    maxID = std::max(maxID, b.ID);
    for (const auto& t : textboxes)  maxID = std::max(maxID, t.ID);
    for (const auto& d : dialogues)  maxID = std::max(maxID, d.ID);
    for (const auto& dp : displays)  maxID = std::max(maxID, dp.ID);
    nextUIID = std::max(1, maxID + 1);
}

/**
 * @brief       Returns a reference to a UIGroup by name.
 * @param name  Group name to search for.
 * @return      UIGroup& Reference to the matching group.
 */

// Returns a reference to the UIGroup with the given name.
// Throws std::out_of_range if no group with the given name is found.
UIGroup& GuiSystem::GetGroup(const std::string& name) 
{
    for (UIGroup& group : m_groups) 
    {
        if (group.groupName == name)
        {
            return group;
        }
    }
    throw std::out_of_range("GuiSystem::GetGroup: group not found: " + name);
}

/**
 * @brief           Sets the current frame (state) of an ImageSheet-based display by UI ID.
 *                  Only valid for Display elements with UIRenderType::ImageSheet.
 *                  Logs an error if the ID is not an ImageSheet display.
 * @param ID        The display's unique UI ID.
 * @param frame     Desired frame/state ID in the sheet.
 * @return true     If the frame was updated successfully.
 * @return false    If display not found or is not ImageSheet.
 */

bool GuiSystem::SetDisplaySheetState(int ID, unsigned int frame) 
{
    for (auto& display : displays)
    {
        if (display.ID == ID)
        {
            if (display.m_renderData.GetRenderType() != UIRenderType::ImageSheet)
            {
                std::cerr << "Tried to set UI element " << ID << " sheet frame, but it is not imagesheet\n";
                return false;
            }

            display.m_renderData.GetImageSheetData().SetFrame(frame);
            return true;
        }
    }
    return false;
}


bool GuiSystem::GetDisplayQuad(int ID, Vec2& v0, Vec2& v1, Vec2& v2, Vec2& v3) {
    for (auto& display : displays)
    {
        if (display.ID != ID) continue;

        if (display.m_renderData.GetRenderType() != UIRenderType::CustomQuad)
        {
            std::cerr << "Tried to get quad data from element " << ID << " but its not quad\n";
            return false;
        }

        UICustomQuadRender quad = display.m_renderData.GetQuadData();
        v0 = quad.v0;
        v1 = quad.v1;
        v2 = quad.v2;
        v3 = quad.v3;
        return true;
    }
    return false;
}

bool GuiSystem::SetDisplayQuad(int ID, Vec2 v0, Vec2 v1, Vec2 v2, Vec2 v3) {
    for (auto& display : displays)
    {
        if (display.ID != ID) continue;

        if (display.m_renderData.GetRenderType() != UIRenderType::CustomQuad)
        {
            std::cerr << "Tried to get quad data from element " << ID << " but its not quad\n";
            return false;
        }

        UICustomQuadRender& quad = display.m_renderData.GetQuadData();
        
        quad.v0 = v0;
        quad.v1 = v1;
        quad.v2 = v2;
        quad.v3 = v3;
        return true;
    }
    return false;
}