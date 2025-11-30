/* ========================================================
 * @file        GuiSystem.cpp
 * @author		Manfred Sim | manfred.sim@digipen.edu
 * @date        30 OCT 2025
 *
 * @brief
 *
 * Implements the GuiSystem, which provides a in-game editor 
 * for UI elements (inside the ImGui editor).
 *
 * Copyright � 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "GuiSystem.h"
#include "GameStateManager.h"
#include "CoreEngine.h"
#include "glapp.h"
#ifndef NO_IMGUI
#include "ImGuiSystem.h"
#endif
#include "SceneManager.h"
#include <FilePath.h>

AudioManager* GuiSystem::audio = nullptr;

// Settings
static float settingsVolume = 1.f;
static const float volStep = 0.05f;
static const int settingsBarFillID = 7; // to match display ID in settings.json
static const float settingsBarMaxW = 1000.f;
bool settingsNeedSync = false;

// ==============================
// UI_RenderData & variants
// ==============================

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

UIRenderType UI_RenderData::GetRenderType() const {
    if (!data) { return UIRenderType::None; }
    return data->GetType();
}

UIShapeRender& UI_RenderData::GetShapeData() {
    if (data->GetType() != UIRenderType::Shape) { throw std::runtime_error("trying to get shape from ui button but is not shape"); }
    return *static_cast<UIShapeRender*>(data.get());
}

const UIShapeRender& UI_RenderData::GetShapeData() const {
    if (data->GetType() != UIRenderType::Shape) { throw std::runtime_error("trying to get shape from ui button but is not shape"); }
    return *static_cast<const UIShapeRender*>(data.get());
}

UICustomQuadRender& UI_RenderData::GetQuadData() {
    if (data->GetType() != UIRenderType::CustomQuad) { throw std::runtime_error("trying to get shape from ui button but is not shape"); }
    return *static_cast<UICustomQuadRender*>(data.get());
}

const UICustomQuadRender& UI_RenderData::GetQuadData() const {
    if (data->GetType() != UIRenderType::CustomQuad) { throw std::runtime_error("trying to get shape from ui button but is not shape"); }
    return *static_cast<const UICustomQuadRender*>(data.get());
}

UIImageRender& UI_RenderData::GetImageData() {
    if (data->GetType() != UIRenderType::Image) { throw std::runtime_error("trying to get shape from ui button but is not shape"); }
    return *static_cast<UIImageRender*>(data.get());
}

const UIImageRender& UI_RenderData::GetImageData() const {
    if (data->GetType() != UIRenderType::Image) { throw std::runtime_error("trying to get shape from ui button but is not shape"); }
    return *static_cast<const UIImageRender*>(data.get());
}

UIImageSheetRender& UI_RenderData::GetImageSheetData() {
    if (data->GetType() != UIRenderType::ImageSheet) { throw std::runtime_error("trying to get shape from ui button but is not shape"); }
    return *static_cast<UIImageSheetRender*>(data.get());
}

const UIImageSheetRender& UI_RenderData::GetImageSheetData() const {
    if (data->GetType() != UIRenderType::ImageSheet) { throw std::runtime_error("trying to get shape from ui button but is not shape"); }
    return *static_cast<const UIImageSheetRender*>(data.get());
}

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

void GuiSystem::SetAudioManager(AudioManager* audioMgr) {
    audio = audioMgr;
}

// Helper to adjust volume based on settings
static void ApplyNewVolume(float v)
{
    FMOD::Channel* ch = AudioManager::Get().GetCurrentMusicChannel();
    if (ch)
    {
        ch->setVolume(std::clamp(v, 0.f, 1.f));
    }
}

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

void GuiSystem::AddTextBox(const TextBoxConfig& cfg)
{
    TextBoxConfig copy = cfg;
    copy.basePos = cfg.pos;
    copy.baseScale = cfg.scale;
    if (copy.ID <= 0) copy.ID = nextUIID++;
    else nextUIID = std::max(nextUIID, copy.ID + 1);
    textboxes.push_back(copy);
}

void GuiSystem::AddDialogue(const DialogueConfig& cfg)
{
    DialogueConfig copy = cfg;
    if (copy.ID <= 0) copy.ID = nextUIID++;
    else nextUIID = std::max(nextUIID, copy.ID + 1);
    dialogues.push_back(std::move(copy));
}

void GuiSystem::AddDisplay(const DisplayConfig& cfg)
{
    DisplayConfig copy = cfg;
    copy.basePos = cfg.pos;
    copy.baseSize = cfg.size;
    if (copy.ID <= 0) copy.ID = nextUIID++;
    else nextUIID = std::max(nextUIID, copy.ID + 1);
    displays.push_back(std::move(copy));
}

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

void GuiSystem::SetEditMode(bool isEditing)
{
    editMode = isEditing;
}

bool GuiSystem::GetEditMode() const
{
    return editMode;
}

void GuiSystem::Update(float dt)
{
    // --- SPACE advances dialogue when NOT editing ---
    static bool spaceHandled = false;
    const bool spacePressed = Input::IsKeyPressed(GLFW_KEY_SPACE);
    const bool spaceHeld = Input::IsKeyHeld(GLFW_KEY_SPACE);

    if (!editMode && spacePressed && !spaceHandled)
    {
        for (auto& d : dialogues)
        {
            if (!d.isVisible) continue;
            TextSystem::TextComponent* comp = d.textComp ? d.textComp : d.owned.get();
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
                    comp->StartTyping();
                }
            }
        }
        spaceHandled = true;
    }
    if (!spaceHeld) spaceHandled = false;

    // Mouse (convert to GameView space if ImGui is active)
    Vec2 mouse = Input::GetMousePosition();
    GuiSystem::GetMouseImgui(mouse);

    // Render setup (UI in screen space)
    Render& render = Render::GetInstance();              // <-- restored local renderer
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

            SceneManager& sm = SceneManager::Get();

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
                TextSystem::UpdateTyping(*comp, dt, 1.f);
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
            const float nameScale = d.textBox.scale * 0.9f;
            const float lineH = render.CalculateTextSize("A", d.textBox.scale, "Marker").y;
            render.DrawText(speakerName, drawX, drawY + (lineH * 1.5f), nameScale, drawColor, "Marker");

            // Word-wrap visible text
            const float maxWidth = (defaultWidth / 2) * uiScale;
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
            render.DrawCustomQuad(data.v0, data.v1, data.v2, data.v3, data.color);
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

void GuiSystem::Shutdown() {
    buttons.clear();
    textboxes.clear();
    dialogues.clear();
    displays.clear();
    nextUIID = 1;
    //audio = nullptr;
}

std::string ButtonShapeToString(ButtonShape shape) {
    switch (shape) {
    case ButtonShape::Rounded:      return "Rounded";
    case ButtonShape::Rectangle:    return "Rectangle";
    case ButtonShape::Circle:       return "Circle";
    default:                        return "Rectangle";
    }
}

ButtonShape StringToButtonShape(const std::string& s) {
    if (s == "Rounded") return ButtonShape::Rounded;
    if (s == "Circle")  return ButtonShape::Circle;
    return ButtonShape::Rectangle;
}

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

void GuiSystem::Clear()
{
    buttons.clear();
    textboxes.clear();
    dialogues.clear();
    displays.clear();
    m_groups.clear();
    nextUIID = 1;
}

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

void GuiSystem::ToggleAllTextboxes()
{
    if (textboxes.empty()) return;
    bool anyVisible = false;
    for (auto& t : textboxes) if (t.isVisible) { anyVisible = true; break; }
    for (auto& t : textboxes) t.isVisible = !anyVisible;
}

void GuiSystem::ToggleAllDialogues()
{
    if (dialogues.empty()) return;
    bool anyVisible = false;
    for (auto& d : dialogues) if (d.isVisible) { anyVisible = true; break; }
    for (auto& d : dialogues) d.isVisible = !anyVisible;
}

void GuiSystem::ToggleAllDisplays()
{
    if (displays.empty()) return;
    bool anyVisible = false;
    for (auto& dp : displays) if (dp.isVisible) { anyVisible = true; break; }
    for (auto& dp : displays) dp.isVisible = !anyVisible;
}

void GuiSystem::DrawStateWindow(Render& render)
{
    if (!stateWindowActive) return;

    const bool prevCam = render.useCamera;
    render.useCamera = false;

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

void GuiSystem::GetMouseImgui(Vec2& mouse)
{
#ifndef NO_IMGUI
    if (!ImGuiSystem::guiEnabled) {
        // Fullscreen mouse coords are already correct
    }
    else {
        if (mouse.x < m_gameViewRect.x || mouse.x > m_gameViewRect.x + m_gameViewRect.width ||
            mouse.y < m_gameViewRect.y || mouse.y > m_gameViewRect.y + m_gameViewRect.height)
        {
            mouse.x = -9999.f;
            mouse.y = -9999.f;
        }
        else {
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

void GuiSystem::RegisterAction(const std::string& name, ActionFunction func) {
    if (actionRegistry.contains(name)) { std::cerr << "Button function " << name << " already registered...\n"; }
    actionRegistry[name] = func;
}

bool GuiSystem::CallAction(const std::string& name, const nlohmann::json& params) {
    auto it = actionRegistry.find(name);

    if (it == actionRegistry.end()) {
        std::cerr << "[GuiSystem] Button action '" << name << "' called but not registered!\n";
        return false;
    }

    try {
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
void GuiSystem::ReseedNextUIID()
{
    int maxID = 0;
    for (const auto& b : buttons)    maxID = std::max(maxID, b.ID);
    for (const auto& t : textboxes)  maxID = std::max(maxID, t.ID);
    for (const auto& d : dialogues)  maxID = std::max(maxID, d.ID);
    for (const auto& dp : displays)  maxID = std::max(maxID, dp.ID);
    nextUIID = std::max(1, maxID + 1);
}

UIGroup& GuiSystem::GetGroup(const std::string& name) {
    for (UIGroup& group : m_groups) {
        if (group.groupName == name)
        {
            return group;
        }
    }
}

bool GuiSystem::SetDisplaySheetState(int ID, unsigned int frame) {
    
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