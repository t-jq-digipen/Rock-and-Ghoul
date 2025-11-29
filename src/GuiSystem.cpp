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

void GuiSystem::SetAudioManager(AudioManager* audioMgr) {
    audio = audioMgr;
}

void GuiSystem::Initialize() {
    buttons.clear();
    textboxes.clear();
    dialogues.clear();
    nextUIID = 1; // start fresh
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

    SceneManager& sm = SceneManager::Get();
    if (!sm.inEditMode && spacePressed && !spaceHandled)
    {
        for (auto& d : dialogues)
        {
            if (!d.isVisible) continue;
            TextSystem::TextComponent* comp = d.textComp ? d.textComp : d.owned.get();

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
    Render& renderer = Render::GetInstance();
    const bool prevCam = renderer.useCamera;
    renderer.useCamera = false;

    // UI scaling against 1920x1080 virtual space
    const float defaultWidth = 1920.f;
    const float defaultHeight = 1080.f;
    const float scaleX = GLApp::width / defaultWidth;
    const float scaleY = GLApp::height / defaultHeight;
    const float uiScale = std::min(scaleX, scaleY);

    // Build draw order: ALL UI elements sorted by ascending ID
    auto drawList = BuildSortedDrawList();

    // Draw UI elements per type
    auto drawButtonAt = [&](int i)
        {
            auto& cfg = buttons[i];
            if (!cfg.isVisible) return;

            cfg.size = cfg.baseSize * uiScale;
            cfg.textScale = cfg.baseTextScale * uiScale;

        switch (cfg.shape) 
        {
        case ButtonShape::Rectangle:
            render.DrawRect(cfg.pos, cfg.size.x, cfg.size.y, drawColor);
            break;
        case ButtonShape::Circle: 
        {
            float radius = std::min(cfg.size.x, cfg.size.y) * 0.5f;
            render.DrawCircle(cfg.pos, radius, drawColor, 48);
            break;
        }
        case ButtonShape::Rounded: 
        {
            float corner = std::min(cfg.size.x, cfg.size.y) * 0.5f;
            render.DrawRoundedRect(cfg.pos, cfg.size.x, cfg.size.y, corner, drawColor);

            //idk where to test LOL
            //render.DrawCustomQuad({ 100 * scaleX,200 * scaleY }, { 300 * scaleX,200 * scaleY }, { 350 * scaleX,250 * scaleY }, { 150 * scaleX,250 * scaleY }, {1.f,0.f,0.f});
      
            break;
        }
        }

        float textScale = cfg.textScale;
        Vec2 textSize = render.CalculateTextSize(cfg.label, textScale, "Marker");

        // text horizontally and vertically on the button
        float textX = cfg.pos.x - textSize.x * 0.5f;
        float textY = cfg.pos.y - textSize.y * 0.5f;

        //small vertical offset
        textY += textSize.y * 0.15f;

        // UPDATED: Added font parameter to DrawText
        render.DrawText(cfg.label, textX, textY, textScale, { 1, 1, 1 }, "Marker");

#if 1 //EDITORMACRO
        if (!SceneManager::Get().inEditMode) {
#endif
            if (hover && Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
                if (audio) audio->PlayButtonSound(cfg.soundType);
                if (cfg.onClick) cfg.onClick();
            }
#if 1 //EDITORMACRO
        }
#endif
    }

    for (auto& tb : textboxes)
    {
        if (!tb.isVisible) continue;
        tb.pos.x = tb.basePos.x * scaleX;
        tb.pos.y = tb.basePos.y * scaleY;
        tb.scale = tb.baseScale * uiScale;
        Vec2 textSize = render.CalculateTextSize(tb.text, tb.scale, tb.fontKey);

        float drawX = tb.pos.x;
        float drawY = tb.pos.y;

        if (tb.centered)
        {
            drawX -= textSize.x * 0.5f;
            drawY -= textSize.y * 0.5f;
            // drawY += textSize.y * 0.15f;
        }

        // UPDATED: Added font parameter to DrawText
        render.DrawText(tb.text, drawX, drawY, tb.scale, tb.color, tb.fontKey);
    }

    SceneManager& sm = SceneManager::Get();
    for (auto& d : dialogues)
    {
        if (!d.isVisible) continue;
        TextSystem::TextComponent* comp = d.textComp ? d.textComp : d.owned.get();
        if (!comp) continue;

        // scale skin
        d.textBox.pos.x = d.textBox.basePos.x * scaleX;
        d.textBox.pos.y = d.textBox.basePos.y * scaleY;
        d.textBox.scale = d.textBox.baseScale * uiScale;

        std::string vis;
        const std::string& fullLine = comp->GetCurrentLine(comp->GetCurrentLineIndex());

        if (sm.inEditMode)
        {
            // In edit mode: Show full current line (no typing)
            if (comp->CheckIfTyping()) comp->StopTyping();

            const auto& lines = comp->GetLines();
            if (lines.empty()) continue;

            int idx = comp->GetCurrentLineIndex();
            if (idx < 0) idx = 0;
            if (idx >= (int)lines.size()) idx = (int)lines.size() - 1;
            vis = lines[idx];
        }
        else
        {
            // In play mode: Advance with the typewriter
            TextSystem::UpdateTyping(*comp, dt, 1.f);

            const auto& lines = comp->GetLines();
            if (lines.empty()) continue;

            int idx = comp->GetCurrentLineIndex();
            if (idx < 0) idx = 0;
            if (idx >= (int)lines.size()) idx = (int)lines.size() - 1;
            vis = comp->CheckIfTyping() ? TextSystem::GetVisibleText(*comp) : lines[idx];
        }

        Colour3 drawColor = d.textBox.color;
        Vec2 textSize = render.CalculateTextSize(fullLine, d.textBox.scale, "Marker");
        float drawX = d.textBox.pos.x;
        float drawY = d.textBox.pos.y;
        if (d.textBox.centered) 
        { 
            drawX -= textSize.x * 0.5f; 
            drawY -= textSize.y * 0.5f; 
        }

        // Get speaker name to display above line
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

            Colour3 drawColor = hover
                ? Colour3{ cfg.color.r + 0.2f, cfg.color.g + 0.2f, cfg.color.b + 0.2f }
            : cfg.color;

            switch (cfg.shape) {
            case ButtonShape::Rectangle:
                render.DrawRect(cfg.pos, cfg.size.x, cfg.size.y, drawColor);
                break;
            case ButtonShape::Circle: {
                float radius = std::min(cfg.size.x, cfg.size.y) * 0.5f;
                render.DrawCircle(cfg.pos, radius, drawColor, 48);
                break;
            }
            case ButtonShape::Rounded: {
                float corner = std::min(cfg.size.x, cfg.size.y) * 0.5f;
                render.DrawRoundedRect(cfg.pos, cfg.size.x, cfg.size.y, corner, drawColor);
                break;
            }
            }

            const float textScale = cfg.textScale;
            Vec2 textSize = render.CalculateTextSize(cfg.label, textScale, "Marker");

            // center text on button
            float textX = cfg.pos.x - textSize.x * 0.5f;
            float textY = cfg.pos.y - textSize.y * 0.5f;
            textY += textSize.y * 0.15f; // small vertical offset

            render.DrawText(cfg.label, textX, textY, textScale, { 1, 1, 1 }, "Marker");

            if (hover && Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
                if (audio) audio->PlayButtonSound(cfg.soundType);
                if (cfg.onClick) cfg.onClick();
            }
        };

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
        };

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

            static bool sPrevEditMode = true;
            if (sPrevEditMode && !sm.inEditMode) 
            {
                if (!lines[idx].empty()) 
                {
                    comp->StartTyping();
                }
            }
            sPrevEditMode = sm.inEditMode;

            const std::string& fullLine = comp->GetCurrentLine(idx);
            std::string vis;

            if (sm.inEditMode) 
            {
                // In edit mode: show full line
                comp->SetIsTyping(false);
                vis = lines[idx];
            }
            else 
            {
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
        };

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
        default: break;
        }
    }

    // --- State window over UI ---
    if (stateWindowActive) 
    {
        DrawStateWindow(render);
    }
    /*
    if (editMode) {
        // Background panel
        render.DrawRect({ 250, 540 }, 500, 1000, { 0.15f, 0.15f, 0.15f });
        // UPDATED: Added font parameter to DrawText
        render.DrawText("UI ELEMENTS", 50, 900, 1.0f, { 1,1,1 }, "Marker");

        float y = 850.f;
        bool buttondeleted = false;
        // --- List Buttons ---
        for (int i = 0; i < (int)buttons.size(); ++i)
        {
            auto& b = buttons[i];
            bool isSelected = (selectedType == SelectedType::Button && selectedIndex == i);

            Colour3 color = isSelected ? Colour3{ 0.9f,0.6f,0.2f } : Colour3{ 0.25f,0.25f,0.25f };
            render.DrawRect({ 150, y }, 600, 50, color);
            // UPDATED: Added font parameter to DrawText
            render.DrawText("Button: " + b.label, 20, y - 10, 0.5f, { 1,1,1 }, "Marker");
            //delete box position
            Vec2 delPos = { 150 + 300 + 20, y }; // right of the entry
            float delW = 40.f, delH = 40.f;

            // draw delete box
            // UPDATED: Added font parameter to DrawText
            render.DrawText("X", delPos.x - 10, y - 10, 1.0f, { 1,0,0 }, "Marker");

            // if clicked, delete button
            if (render.IsAreaTouched(mouse, delPos, delW, delH) &&
                Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
            {
                if (buttons.size() > 0) {
                    buttons.erase(buttons.begin() + i);

                    // fix selection index after delete
                    selectedIndex = -1;
                    selectedType = SelectedType::None;
                    buttondeleted = true;
                    return; // break to avoid invalid iterator
                }
            }

            if (render.IsAreaTouched(mouse, { 150, y }, 300, 50) &&
                Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
            {
                selectedType = SelectedType::Button;
                selectedIndex = i;
            }

            y -= 60.f;
            if (buttondeleted)
                return;
        }

        // --- List TextBoxes ---
        bool textDeleted = false;
        for (int i = 0; i < (int)textboxes.size(); ++i)
        {
            auto& t = textboxes[i];
            int textIndex = (int)buttons.size() + i;
            bool isSelected = (selectedType == SelectedType::TextBox && selectedIndex == textIndex);

            Colour3 color = isSelected ? Colour3{ 0.9f,0.6f,0.2f } : Colour3{ 0.25f,0.25f,0.25f };
            render.DrawRect({ 150, y }, 600, 50, color);
            // UPDATED: Added font parameter to DrawText
            render.DrawText("Text: " + t.text, 20, y - 10, 0.5f, { 1,1,1 }, "Marker");
            //delete box position
            Vec2 delPos = { 150.f + 300.f + 20.f, y }; // right of the entry
            float delW = 40.f, delH = 40.f;

            // draw delete box
            // UPDATED: Added font parameter to DrawText
            render.DrawText("X", delPos.x - 10, y - 10, 1.0f, { 1,0,0 }, "Marker");

            // if clicked, delete button
            if (render.IsAreaTouched(mouse, delPos, delW, delH) &&
                Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
            {
                if (textboxes.size() > 0) {
                    textboxes.erase(textboxes.begin() + i);

                    // fix selection index after delete
                    selectedIndex = -1;
                    selectedType = SelectedType::None;
                    textDeleted = true;
                    return; // break to avoid invalid iterator
                }
            }

            if (render.IsAreaTouched(mouse, { 150, y }, 300, 50) &&
                Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
            {
                selectedType = SelectedType::TextBox;
                selectedIndex = textIndex;
            }

            y -= 60.f;
            if (textDeleted)
                return;
        }


        // List Dialogues
        bool dialogueDeleted = false;
        for (int i = 0; i < (int)dialogues.size(); ++i) {
            int idx = (int)buttons.size() + (int)textboxes.size() + i;
            bool sel = (selectedType == SelectedType::Dialogue && selectedIndex == idx);
            render.DrawRect({ 150, y }, 600, 50, sel ? Colour3{ 0.9f,0.6f,0.2f } : Colour3{ 0.25f,0.25f,0.25f });

            // label from current line head (preview)
            TextSystem::TextComponent* comp = dialogues[i].textComp ? dialogues[i].textComp : dialogues[i].owned.get();
            std::string label = "Dialogue";
            if (comp && !comp->GetLines().empty()) {
                const std::string& ln = comp->GetLines()[comp->GetCurrentLineIndex()];
                label += ": " + (ln.size() > 24 ? (ln.substr(0, 24) + "...") : ln);
            }
            render.DrawText(label, 20, y - 10, 0.5f, { 1,1,1 }, "Marker");

            //delete box position
            Vec2 delPos = { 150.f + 300.f + 20.f, y }; // right of the entry
            float delW = 40.f, delH = 40.f;

            // draw delete box
            // UPDATED: Added font parameter to DrawText
            render.DrawText("X", delPos.x - 10, y - 10, 1.0f, { 1,0,0 }, "Marker");

            // if clicked, delete button
            if (render.IsAreaTouched(mouse, delPos, delW, delH) &&
                Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
            {
                if (dialogues.size() > 1) {
                    dialogues.erase(dialogues.begin() + i);

                    // fix selection index after delete
                    selectedIndex = -1;
                    selectedType = SelectedType::None;
                    dialogueDeleted = true;
                    return; // break to avoid invalid iterator
                }
            }

            if (render.IsAreaTouched(mouse, { 150, y }, 300, 50) && Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
                selectedType = SelectedType::Dialogue; selectedIndex = idx;
            }
            y -= 60;
            if (dialogueDeleted)
                return;
        }


        if (selectedType != SelectedType::None)
        {
            // UPDATED: Added font parameter to DrawText
            render.DrawText("PROPERTIES", 50, 400, 1.2f, { 1,1,1 }, "Marker");
            y = 360;

            if (selectedType == SelectedType::Button && selectedIndex < buttons.size())
            {
                auto& b = buttons[selectedIndex];

                static bool editingLabel = false;
                static std::string tempLabel;

                // press Enter to start editing the label
                if (Input::IsKeyPressed(GLFW_KEY_ENTER)) {
                    if (!editingLabel) {
                        editingLabel = true;
                        tempLabel = b.label;
                    }
                    else {
                        // confirm rename
                        b.label = tempLabel.empty() ? b.label : tempLabel;
                        editingLabel = false;
                    }
                }

                if (editingLabel) {
                    // UPDATED: Added font parameter to DrawText
                    render.DrawText("Editing Label: " + tempLabel + "_", 50, y, 0.5f, { 1, 1, 0 }, "Marker");
                }
                else {
                    // UPDATED: Added font parameter to DrawText
                    render.DrawText("Label: " + b.label, 50, y, 0.5f, { 1, 1, 1 }, "Marker");
                }
                y -= 30;

                if (editingLabel) {
                    // letters (A–Z)
                    for (int key = GLFW_KEY_A; key <= GLFW_KEY_Z; ++key) {
                        if (Input::IsKeyPressed(key)) {
                            bool shift = Input::IsKeyHeld(GLFW_KEY_LEFT_SHIFT) || Input::IsKeyHeld(GLFW_KEY_RIGHT_SHIFT);
                            char c = static_cast<char>('a' + (key - GLFW_KEY_A));
                            if (shift) c = static_cast<char>(toupper(c));
                            tempLabel.push_back(c);
                        }
                    }

                    // numbers (0–9)
                    for (int key = GLFW_KEY_0; key <= GLFW_KEY_9; ++key) {
                        if (Input::IsKeyPressed(key)) {
                            tempLabel.push_back(static_cast<char>('0' + (key - GLFW_KEY_0)));
                        }
                    }

                    // space
                    if (Input::IsKeyPressed(GLFW_KEY_SPACE))
                        tempLabel.push_back(' ');

                    // backspace
                    if (Input::IsKeyPressed(GLFW_KEY_BACKSPACE) && !tempLabel.empty())
                        tempLabel.pop_back();
                }
                // UPDATED: Added font parameter to DrawText
                render.DrawText("Pos: (" + std::to_string((int)b.pos.x) + ", " +
                    std::to_string((int)b.pos.y) + ")", 50, y, 0.5f, { 1,1,1 }, "Marker");
                y -= 30;
                // UPDATED: Added font parameter to DrawText
                render.DrawText("Size: (" + std::to_string((int)b.size.x) + ", " +
                    std::to_string((int)b.size.y) + ")", 50, y, 0.5f, { 1,1,1 }, "Marker");
                y -= 30;

                // UPDATED: Added font parameter to DrawText
                render.DrawText("Text Size: (" + std::to_string(b.textScale) + ")", 50, y, 0.5f, { 1,1,1 }, "Marker");
                y -= 30;

                // Width Control
                if (Input::IsKeyHeld(GLFW_KEY_RIGHT_BRACKET)) {    // ]
                    b.baseSize.x += 4.0f;   // increase width
                }
                if (Input::IsKeyHeld(GLFW_KEY_LEFT_BRACKET)) {     // [
                    b.baseSize.x = std::max(20.0f, b.baseSize.x - 4.0f);   // decrease width
                }

                // Height Control
                if (Input::IsKeyHeld(GLFW_KEY_APOSTROPHE)) {       // '
                    b.baseSize.y += 2.0f;   // increase height
                }
                if (Input::IsKeyHeld(GLFW_KEY_SEMICOLON)) {        // ;
                    b.baseSize.y = std::max(10.0f, b.baseSize.y - 2.0f);   // decrease height
                }

                // maybe arrow keys to tweak position
                if (Input::IsKeyHeld(GLFW_KEY_UP))    b.basePos.y += 2;
                if (Input::IsKeyHeld(GLFW_KEY_DOWN))  b.basePos.y -= 2;
                if (Input::IsKeyHeld(GLFW_KEY_LEFT))  b.basePos.x -= 2;

                if (Input::IsKeyHeld(GLFW_KEY_O))     // smaller text
                    b.baseTextScale = std::max(0.2f, b.baseTextScale - 0.02f);
                if (Input::IsKeyHeld(GLFW_KEY_P))     // bigger text
                    b.baseTextScale = std::min(3.0f, b.baseTextScale + 0.02f);

                if (Input::IsKeyHeld(GLFW_KEY_RIGHT)) b.basePos.x += 2;

                // SELECT SHAPES
                float boxY = y;  // current vertical cursor
                // UPDATED: Added font parameter to DrawText
                render.DrawText("Shape:", 50, boxY, 0.5f, { 1,1,1 }, "Marker");
                boxY -= 40;

                float startX = 80;
                float gap = 140;
                float boxW = 100, boxH = 40;

                struct ShapeOption { std::string name; ButtonShape type; Colour3 color; };
                std::vector<ShapeOption> options = {
                    {"Rect", ButtonShape::Rectangle, {0.3f, 0.5f, 0.9f}},
                    {"Curved",   ButtonShape::Rounded,   {0.3f, 0.9f, 0.5f}},
                    {"Circle",    ButtonShape::Circle,    {0.9f, 0.5f, 0.3f}}
                };

                for (size_t i = 0; i < options.size(); ++i)
                {
                    const auto& opt = options[i];
                    Colour3 c = (b.shape == opt.type)
                        ? Colour3{ opt.color.r + 0.3f, opt.color.g + 0.3f, opt.color.b + 0.3f }
                    : opt.color;

                    Vec2 boxPos = { startX + i * gap, boxY };
                    render.DrawRect(boxPos, boxW, boxH, c);
                    // UPDATED: Added font parameter to DrawText
                    render.DrawText(opt.name, boxPos.x - 35, boxPos.y - 10, 0.5f, { 1,1,1 }, "Marker");

                    // check click
                    if (render.IsAreaTouched(mouse, boxPos, boxW, boxH) &&
                        Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
                    {
                        b.shape = opt.type;  // switch shape
                    }
                }
                y = boxY - 50;  // move property panel cursor below shapes

                //COLOR
                // UPDATED: Added font parameter to DrawText
                render.DrawText("Color (0-255):", 50, y, 0.5f, { 1,1,1 }, "Marker");
                y -= 30;

                //store RGB as integer temp for display/edit
                int rVal = static_cast<int>(b.color.r * 255.0f);
                int gVal = static_cast<int>(b.color.g * 255.0f);
                int bVal = static_cast<int>(b.color.b * 255.0f);

                // UPDATED: Added font parameter to DrawText
                render.DrawText("R:", 60, y, 0.5f, { 1,0,0 }, "Marker");
                render.DrawText(std::to_string(rVal), 100, y, 0.5f, { 1,1,1 }, "Marker");
                render.DrawText("G:", 200, y, 0.5f, { 0,1,0 }, "Marker");
                render.DrawText(std::to_string(gVal), 240, y, 0.5f, { 1,1,1 }, "Marker");
                render.DrawText("B:", 340, y, 0.5f, { 0,0,1 }, "Marker");
                render.DrawText(std::to_string(bVal), 380, y, 0.5f, { 1,1,1 }, "Marker");

                // simple keyboard-based editing
                static int channel = 0; // 0=R,1=G,2=B
                if (Input::IsKeyPressed(GLFW_KEY_TAB)) {
                    channel = (channel + 1) % 3;
                }

                int* target = (channel == 0) ? &rVal : (channel == 1) ? &gVal : &bVal;
                if (Input::IsKeyHeld(GLFW_KEY_EQUAL))    *target = std::min(255, *target + 1);
                if (Input::IsKeyHeld(GLFW_KEY_MINUS))  *target = std::max(0, *target - 1);

                // apply back to the button color
                b.color.r = rVal / 255.0f;
                b.color.g = gVal / 255.0f;
                b.color.b = bVal / 255.0f;

                // draw indicator circle beside the active label
                float indicatorY = y + 5; // vertically align with text
                Colour3 c = (channel == 0) ? Colour3{ 1,0,0 } :
                    (channel == 1) ? Colour3{ 0,1,0 } :
                    Colour3{ 0,0,1 };

                float circleX = 40.f + channel * 140.f; // 0R, 140G, 280B
                render.DrawCircle({ circleX, indicatorY }, 6.f, c, 24);

                y -= 40;
            }

            else if (selectedType == SelectedType::TextBox && selectedIndex - buttons.size() < textboxes.size())
            {
                int idx = selectedIndex - (int)buttons.size();
                auto& t = textboxes[idx];

                static bool editingText = false;
                static std::string tempText;

                if (Input::IsKeyPressed(GLFW_KEY_ENTER))
                {
                    if (!editingText)
                    {
                        editingText = true;
                        tempText = t.text;
                    }
                    else
                    {
                        // confirm and exit
                        t.text = tempText.empty() ? t.text : tempText;
                        editingText = false;
                    }
                }
                if (editingText)
                    // UPDATED: Added font parameter to DrawText
                    render.DrawText("Editing Text: " + tempText + "_", 50, y, 0.5f, { 1,1,0 }, "Marker");
                else
                    // UPDATED: Added font parameter to DrawText
                    render.DrawText("Text: " + t.text, 50, y, 0.5f, { 1,1,1 }, "Marker");
                y -= 30;

                if (editingText) {
                    // letters (A–Z)
                    for (int key = GLFW_KEY_A; key <= GLFW_KEY_Z; ++key) {
                        if (Input::IsKeyPressed(key)) {
                            bool shift = Input::IsKeyHeld(GLFW_KEY_LEFT_SHIFT) || Input::IsKeyHeld(GLFW_KEY_RIGHT_SHIFT);
                            char c = static_cast<char>('a' + (key - GLFW_KEY_A));
                            if (shift) c = static_cast<char>(toupper(c));
                            tempText.push_back(c);
                        }
                    }

                    // numbers (0–9)
                    for (int key = GLFW_KEY_0; key <= GLFW_KEY_9; ++key) {
                        if (Input::IsKeyPressed(key)) {
                            tempText.push_back(static_cast<char>('0' + (key - GLFW_KEY_0)));
                        }
                    }

                    // space
                    if (Input::IsKeyPressed(GLFW_KEY_SPACE))
                        tempText.push_back(' ');

                    // backspace
                    if (Input::IsKeyPressed(GLFW_KEY_BACKSPACE) && !tempText.empty())
                        tempText.pop_back();
                }
                // UPDATED: Added font parameter to DrawText
                render.DrawText("Pos: (" + std::to_string((int)t.pos.x) + ", " +
                    std::to_string((int)t.pos.y) + ")", 50, y, 0.5f, { 1,1,1 }, "Marker");
                y -= 30;
                // UPDATED: Added font parameter to DrawText
                render.DrawText("Size: (" + std::to_string(t.scale) + ")", 50, y, 0.5f, { 1,1,1 }, "Marker");
                y -= 30;

                // UPDATED: Added font parameter to DrawText
                render.DrawText("Text Color (0-255):", 50, y, 0.5f, { 1,1,1 }, "Marker");
                y -= 30;

                int rVal = static_cast<int>(t.color.r * 255.0f);
                int gVal = static_cast<int>(t.color.g * 255.0f);
                int bVal = static_cast<int>(t.color.b * 255.0f);

                // UPDATED: Added font parameter to DrawText
                render.DrawText("R:", 60, y, 0.5f, { 1,0,0 }, "Marker");
                render.DrawText(std::to_string(rVal), 100, y, 0.5f, { 1,1,1 }, "Marker");
                render.DrawText("G:", 200, y, 0.5f, { 0,1,0 }, "Marker");
                render.DrawText(std::to_string(gVal), 240, y, 0.5f, { 1,1,1 }, "Marker");
                render.DrawText("B:", 340, y, 0.5f, { 0,0,1 }, "Marker");
                render.DrawText(std::to_string(bVal), 380, y, 0.5f, { 1,1,1 }, "Marker");

                // cycle active color channel using TAB
                static int textColorChannel = 0; // 0=R,1=G,2=B
                if (Input::IsKeyPressed(GLFW_KEY_TAB)) {
                    textColorChannel = (textColorChannel + 1) % 3;
                }

                // modify current channel with = / -
                int* target = (textColorChannel == 0) ? &rVal : (textColorChannel == 1) ? &gVal : &bVal;
                if (Input::IsKeyHeld(GLFW_KEY_EQUAL))    *target = std::min(255, *target + 1);
                if (Input::IsKeyHeld(GLFW_KEY_MINUS))    *target = std::max(0, *target - 1);

                // apply changes
                t.color.r = rVal / 255.0f;
                t.color.g = gVal / 255.0f;
                t.color.b = bVal / 255.0f;

                // draw small indicator for active channel
                float indicatorY = y + 5;
                Colour3 c = (textColorChannel == 0) ? Colour3{ 1,0,0 } :
                    (textColorChannel == 1) ? Colour3{ 0,1,0 } :
                    Colour3{ 0,0,1 };
                float circleX = 40.f + textColorChannel * 140.f;
                render.DrawCircle({ circleX, indicatorY }, 6.f, c, 24);

                y -= 40;
                // maybe arrow keys to tweak position
                if (Input::IsKeyHeld(GLFW_KEY_UP))    t.basePos.y += 2;
                if (Input::IsKeyHeld(GLFW_KEY_DOWN))  t.basePos.y -= 2;
                if (Input::IsKeyHeld(GLFW_KEY_LEFT))  t.basePos.x -= 2;
                if (Input::IsKeyHeld(GLFW_KEY_RIGHT)) t.basePos.x += 2;

                if (Input::IsKeyHeld(GLFW_KEY_O))     // smaller text
                    t.baseScale = std::max(0.2f, t.baseScale - 0.02f);
                if (Input::IsKeyHeld(GLFW_KEY_P))     // bigger text
                    t.baseScale = std::min(3.0f, t.baseScale + 0.02f);
            }

            else if (selectedType == SelectedType::Dialogue && selectedIndex - buttons.size() < dialogues.size())
            {
                int dlgIdx = selectedIndex - ((int)buttons.size() + (int)textboxes.size());
                auto& d = dialogues[dlgIdx];

                static bool editingLine = false;
                std::string editingLineState = editingLine ? "ON" : "OFF";
                static std::string tempText;

                TextSystem::TextComponent* textComp = d.textComp ? d.textComp : d.owned.get();

                if (Input::IsKeyPressed(GLFW_KEY_ENTER))
                {
                    editingLine = !editingLine;
                }

                render.DrawText("Editing Line: " + editingLineState, 50, y, 0.5f, { 1,1,1 }, "Marker");
                y -= 30;

                render.DrawText("Current Line Index: " + std::to_string(textComp->GetCurrentLineIndex()), 50, y, 0.5f, {1,1,1}, "Marker");
                y -= 30;

                // UPDATED: Added font parameter to DrawText
                render.DrawText("Pos: (" + std::to_string((int)d.textBox.pos.x) + ", " +
                    std::to_string((int)d.textBox.pos.y) + ")", 50, y, 0.5f, { 1,1,1 }, "Marker");
                y -= 30;
                // UPDATED: Added font parameter to DrawText
                render.DrawText("Size: (" + std::to_string(d.textBox.scale) + ")", 50, y, 0.5f, { 1,1,1 }, "Marker");
                y -= 30;

                // UPDATED: Added font parameter to DrawText
                render.DrawText("Text Color (0-255):", 50, y, 0.5f, { 1,1,1 }, "Marker");
                y -= 30;

                int rVal = static_cast<int>(d.textBox.color.r * 255.0f);
                int gVal = static_cast<int>(d.textBox.color.g * 255.0f);
                int bVal = static_cast<int>(d.textBox.color.b * 255.0f);

                // UPDATED: Added font parameter to DrawText
                render.DrawText("R:", 60, y, 0.5f, { 1,0,0 }, "Marker");
                render.DrawText(std::to_string(rVal), 100, y, 0.5f, { 1,1,1 }, "Marker");
                render.DrawText("G:", 200, y, 0.5f, { 0,1,0 }, "Marker");
                render.DrawText(std::to_string(gVal), 240, y, 0.5f, { 1,1,1 }, "Marker");
                render.DrawText("B:", 340, y, 0.5f, { 0,0,1 }, "Marker");
                render.DrawText(std::to_string(bVal), 380, y, 0.5f, { 1,1,1 }, "Marker");

                // cycle active color channel using TAB
                static int textColorChannel = 0; // 0=R,1=G,2=B
                if (Input::IsKeyPressed(GLFW_KEY_TAB)) {
                    textColorChannel = (textColorChannel + 1) % 3;
                }

                // modify current channel with = / -
                int* target = (textColorChannel == 0) ? &rVal : (textColorChannel == 1) ? &gVal : &bVal;
                if (Input::IsKeyHeld(GLFW_KEY_EQUAL))    *target = std::min(255, *target + 1);
                if (Input::IsKeyHeld(GLFW_KEY_MINUS))    *target = std::max(0, *target - 1);

                // apply changes
                d.textBox.color.r = rVal / 255.0f;
                d.textBox.color.g = gVal / 255.0f;
                d.textBox.color.b = bVal / 255.0f;

                // draw small indicator for active channel
                float indicatorY = y + 5;
                Colour3 c = (textColorChannel == 0) ? Colour3{ 1,0,0 } :
                    (textColorChannel == 1) ? Colour3{ 0,1,0 } :
                    Colour3{ 0,0,1 };
                float circleX = 40.f + textColorChannel * 140.f;
                render.DrawCircle({ circleX, indicatorY }, 6.f, c, 24);

                y -= 40;

                if (!editingLine)
                {
                    // Move/scale with keys (quick + simple)
                    if (Input::IsKeyHeld(GLFW_KEY_UP))    d.textBox.basePos.y += 2;
                    if (Input::IsKeyHeld(GLFW_KEY_DOWN))  d.textBox.basePos.y -= 2;
                    if (Input::IsKeyHeld(GLFW_KEY_LEFT))  d.textBox.basePos.x -= 2;
                    if (Input::IsKeyHeld(GLFW_KEY_RIGHT)) d.textBox.basePos.x += 2;
                    if (Input::IsKeyHeld(GLFW_KEY_O))     d.textBox.baseScale = std::max(0.2f, d.textBox.baseScale - 0.02f);
                    if (Input::IsKeyHeld(GLFW_KEY_P))     d.textBox.baseScale = std::min(3.0f, d.textBox.baseScale + 0.02f);
                }
                else if (textComp)
                {
                    const auto& lines = textComp->GetLines();
                    int idx = textComp->GetCurrentLineIndex();

                    // Per-key latches so text only changes line once per press
                    static bool leftLatch = false;
                    static bool rightLatch = false;

                    const bool leftHeld = Input::IsKeyPressed(GLFW_KEY_LEFT);
                    const bool rightHeld = Input::IsKeyPressed(GLFW_KEY_RIGHT);

                    const bool leftEdge = !leftLatch && leftHeld;
                    const bool rightEdge = !rightLatch && rightHeld;

                    if (leftEdge) {
                        if (textComp->CheckIfTyping()) {
                            textComp->StopTyping();
                        }
                        else if (idx > 0) {
                            textComp->SetCurrentLineIndex(idx - 1);
                            textComp->StartTyping();
                        }
                    }

                    if (rightEdge) {
                        if (textComp->CheckIfTyping()) {
                            textComp->StopTyping();
                        }
                        else if (idx + 1 < static_cast<int>(lines.size())) {
                            textComp->SetCurrentLineIndex(idx + 1);
                            textComp->StartTyping();
                        }
                    }

                    leftLatch = leftHeld;
                    rightLatch = rightHeld;
                }

            }
        }

        render.DrawRect({ 150, 990 }, 140, 50, { 0.3f, 0.6f, 0.3f });
        render.DrawRect({ 330, 990 }, 140, 50, { 0.3f, 0.6f, 0.3f });
        render.DrawRect({ 510, 990 }, 180, 50, { 0.3f,0.6f,0.3f });
        // UPDATED: Added font parameter to DrawText
        render.DrawText("+Button", 100, 980, 0.5f, { 1,1,1 }, "Marker");
        render.DrawText("+Text", 300, 980, 0.5f, { 1,1,1 }, "Marker");
        render.DrawText("+Dialogue", 470, 980, 0.5f, { 1,1,1 }, "Marker");

        if (render.IsAreaTouched(mouse, { 150, 990 }, 140, 50) &&
            Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            ButtonConfig newBtn;
            newBtn.label = "New Button " + std::to_string((int)buttons.size() + 1);
            newBtn.pos = { 960, 540 };
            newBtn.size = { 200, 100 };
            newBtn.color = { 0.4f, 0.7f, 0.9f };
            newBtn.textScale = 1.0f;

            newBtn.basePos = newBtn.pos;
            newBtn.baseSize = newBtn.size;
            newBtn.baseTextScale = newBtn.textScale;

            buttons.push_back(newBtn);

            selectedType = SelectedType::Button;
            selectedIndex = (int)buttons.size() - 1; // auto-select new
        }

        // +Text
        if (render.IsAreaTouched(mouse, { 330, 990 }, 140, 50) &&
            Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            TextBoxConfig newText;
            newText.text = "New Text " + std::to_string((int)textboxes.size() + 1);
            newText.pos = { 960, 600 };
            newText.scale = 1.2f;
            newText.color = { 0,0,0 };

            newText.basePos = newText.pos;
            newText.baseScale = newText.scale;

            textboxes.push_back(newText);

            selectedType = SelectedType::TextBox;
            selectedIndex = (int)buttons.size() + (int)textboxes.size() - 1; // auto-select new
        }

        // +Dialogue
        if (render.IsAreaTouched(mouse, { 510, 990 }, 180, 50) && Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
            TextBoxConfig newDialogue;
            newDialogue.text = "";
            newDialogue.basePos = { 960, 120 };
            newDialogue.baseScale = 0.9f;
            newDialogue.pos = newDialogue.basePos;
            newDialogue.scale = newDialogue.baseScale;
            newDialogue.color = { 0,0,0 };
            newDialogue.centered = false;

            // default path; user can later edit in UI to reload a different JSON
            const std::string path = "../Assets/Dialogue.json";
            int idx = AddDialogueFromJson(path, newDialogue);
            if (idx >= 0) {
                selectedType = SelectedType::Dialogue;
                selectedIndex = (int)buttons.size() + (int)textboxes.size() + idx;
            }
        }
    }
    */

    float t = SceneManager::Get().GetTransitionProgress();
    if ( t != 1.f)
    {
        float alpha = t;
        Vec2 pos = { GLApp::width * .5f,GLApp::height * .5f };
        //KERVINMAKETHISHAVEALPHA
        render.DrawRect(pos, GLApp::width, GLApp::height, { 0.f,0.f,0.f }, 0.0f, alpha);
    }
   
    // restore camera back to world
    renderer.useCamera = prevCam;
}

void GuiSystem::Shutdown() {
    buttons.clear();
    textboxes.clear();
    dialogues.clear();
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
    if (s == "Circle") return ButtonShape::Circle;
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
    default:                                return "None";
    }
}

ButtonAction StringToButtonAction(const std::string& s)
{
    if (s == "LoadLevel1") return ButtonAction::LoadLevel1;
    if (s == "LoadLevel2") return ButtonAction::LoadLevel2;
    if (s == "LoadMainMenu") return ButtonAction::LoadMainMenu;
    if (s == "Quit") return ButtonAction::Quit;
    if (s == "OpenStateWindow") return ButtonAction::OpenStateWindow;
    if (s == "ToggleAllButtons") return ButtonAction::ToggleAllButtons;
    if (s == "ToggleAllTextboxes") return ButtonAction::ToggleAllTextboxes;
    if (s == "ToggleAllDialogues") return ButtonAction::ToggleAllDialogues;
    return ButtonAction::None;
}

void GuiSystem::Clear() 
{
    buttons.clear();
    textboxes.clear();
    dialogues.clear();
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
    case ButtonAction::LoadLevel1:
        btn.onClick = [this]() { 
            SceneManager::Get().QueueLoadScene("../Assets/saves/level1.json"); 
            SceneManager::Get().SetTransitionTime(2.f);
            };
        break;
    case ButtonAction::LoadMainMenu:
        btn.onClick = [this]() { 
            SceneManager::Get().QueueLoadScene("../Assets/saves/MainMenu.json");
            SceneManager::Get().SetTransitionTime(2.f);
            };
        break;
    case ButtonAction::Quit:
        btn.onClick = [this]() { CoreEngine::GetInstance().Quit(); };
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

void GuiSystem::DrawStateWindow(Render& render)
{
    if (!stateWindowActive) return;

    Render& renderer = Render::GetInstance();
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
        const Vec2 basePos = { 960.f, 560.f };  // virtual center
        const Vec2 baseSize = { 700.f, 550.f };  // virtual size
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
        {"Toggle Buttons",  ButtonAction::ToggleAllButtons} // test toggling of buttons
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

        Colour3 col = hover ? Colour3{ 0.25f + 0.2f, 0.25f + 0.2f, 0.25f + 0.2f }
        : Colour3{ 0.25f, 0.25f, 0.25f };
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
            renderer.useCamera = prevCam;
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
    renderer.useCamera = prevCam;
}

void GuiSystem::GetMouseImgui(Vec2& mouse)
{
    // convert to Game View space
#ifndef NO_IMGUI
    if (!ImGuiSystem::guiEnabled) {
        // Fullscreen mouse coords are already correct, just use them directly
        // No conversion needed!
    }
    else {
        // ImGui is ON convert mouse from screen space to game view space
        if (mouse.x < m_gameViewRect.x || mouse.x > m_gameViewRect.x + m_gameViewRect.width || mouse.y < m_gameViewRect.y || mouse.y > m_gameViewRect.y + m_gameViewRect.height)
        {
            // Mouse is outside game view - invalidate mouse position
            mouse.x = -9999.f;
            mouse.y = -9999.f;
        }
        else {
            // Mouse IS inside game view convert to local coordinates
            mouse.x -= m_gameViewRect.x;
            mouse.y -= m_gameViewRect.y;
            //scale relative to the game view size to match window size
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
    out.reserve(buttons.size() + textboxes.size() + dialogues.size());

    for (int i = 0; i < (int)buttons.size(); ++i) out.push_back({ SelectedType::Button,   i, buttons[i].ID });
    for (int i = 0; i < (int)textboxes.size(); ++i) out.push_back({ SelectedType::TextBox,  i, textboxes[i].ID });
    for (int i = 0; i < (int)dialogues.size(); ++i) out.push_back({ SelectedType::Dialogue, i, dialogues[i].ID });

    std::sort(out.begin(), out.end(), [](const DrawItem& a, const DrawItem& b) {
        // Ascending by ID
        if (a.ID != b.ID) return a.ID < b.ID;
        if (a.type != b.type) return (int)a.type < (int)b.type;
        return a.index < b.index;
        });

    return out;
}