#include "GuiSystem.h"
#include "GameStateManager.h"
#include "CoreEngine.h"
#include "glapp.h"

AudioManager* GuiSystem::audio = nullptr;

void GuiSystem::SetAudioManager(AudioManager* audioMgr) {
    audio = audioMgr;
}

void GuiSystem::Initialize() {
    buttons.clear();
    textboxes.clear();
}

void GuiSystem::AddButton(const ButtonConfig& cfg)
{
    ButtonConfig copy = cfg;
    copy.basePos = cfg.pos;
    copy.baseSize = cfg.size;
    copy.baseTextScale = cfg.textScale;
    buttons.push_back(copy);
}

void GuiSystem::AddTextBox(const TextBoxConfig& cfg)
{
    TextBoxConfig copy = cfg;
    copy.basePos = cfg.pos;
    copy.baseScale = cfg.scale;
    textboxes.push_back(copy);
}

void GuiSystem::SetEditMode(bool isEditing)
{
    editMode = isEditing;
}

bool GuiSystem::GetEditMode() const
{
    return editMode;
}

void GuiSystem::Update(float dt) {
    if (Input::IsKeyPressed(GLFW_KEY_F8))
        editMode = !editMode;
    Vec2 mouse = Input::GetMousePosition();
    Render& render = Render::GetInstance();

    const float uiScale = std::min(GLApp::width / 1920.0f, GLApp::height / 1080.0f);

    for (auto& cfg : buttons) {
        cfg.size = cfg.baseSize * uiScale;
        cfg.textScale = cfg.baseTextScale * uiScale;

        // Scale position relative to baseline resolution
        cfg.pos.x = cfg.basePos.x * (GLApp::width / 1920.0f);
        cfg.pos.y = cfg.basePos.y * (GLApp::height / 1080.0f);
        bool hover = false;
        if (cfg.shape == ButtonShape::Circle) {

            float radius = std::min(cfg.size.x, cfg.size.y) * 0.5f;

            // convert mouse into render-space (bottom-left origin)
            Vec2 mouseRender = { mouse.x, static_cast<float>(GLApp::height) - mouse.y };

            float dist = RG_Distance(mouseRender, cfg.pos);
            hover = dist <= radius;
        }
        else {
            hover = render.IsAreaTouched(mouse, cfg.pos, cfg.size.x, cfg.size.y);
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

        float textScale = cfg.textScale;
        Vec2 textSize = render.CalculateTextSize(cfg.label, textScale);

        // text horizontally and vertically on the button
        float textX = cfg.pos.x - textSize.x * 0.5f;
        float textY = cfg.pos.y - textSize.y * 0.5f;

        //small vertical offset
        textY += textSize.y * 0.15f;

        // UPDATED: Added font parameter to DrawText
        render.DrawText(cfg.label, textX, textY, textScale, { 1, 1, 1 }, "Marker");

        if (hover && Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
            if (audio) audio->PlayButtonSound(cfg.soundType);
            if (cfg.onClick) cfg.onClick();
        }
    }

    const float scaleX = GLApp::width / 1920.0f;
    const float scaleY = GLApp::height / 1080.0f;
    for (auto& tb : textboxes)
    {
        tb.pos.x = tb.basePos.x * scaleX;
        tb.pos.y = tb.basePos.y * scaleY;
        tb.scale = tb.baseScale * uiScale;
        Vec2 textSize = render.CalculateTextSize(tb.text, tb.scale);

        float drawX = tb.pos.x;
        float drawY = tb.pos.y;

        if (tb.centered)
        {
            drawX -= textSize.x * 0.5f;
            drawY -= textSize.y * 0.5f;
            // drawY += textSize.y * 0.15f;
        }

        // UPDATED: Added font parameter to DrawText
        render.DrawText(tb.text, drawX, drawY, tb.scale, tb.color, "Marker");
    }

    if (editMode) {
        // Background panel
        render.DrawRect({ 250, 540 }, 500, 1000, { 0.15f, 0.15f, 0.15f });
        // UPDATED: Added font parameter to DrawText
        render.DrawText("UI ELEMENTS", 50, 900, 1.0f, { 1,1,1 }, "Marker");

        float y = 850;
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
            float delW = 40, delH = 40;

            // draw delete box
            // UPDATED: Added font parameter to DrawText
            render.DrawText("X", delPos.x - 10, y - 10, 1.0f, { 1,0,0 }, "Marker");

            // if clicked, delete button
            if (render.IsAreaTouched(mouse, delPos, delW, delH) &&
                Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
            {
                if (buttons.size() > 1) {
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

            y -= 60;
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
            Vec2 delPos = { 150 + 300 + 20, y }; // right of the entry
            float delW = 40, delH = 40;

            // draw delete box
            // UPDATED: Added font parameter to DrawText
            render.DrawText("X", delPos.x - 10, y - 10, 1.0f, { 1,0,0 }, "Marker");

            // if clicked, delete button
            if (render.IsAreaTouched(mouse, delPos, delW, delH) &&
                Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT))
            {
                if (textboxes.size() > 1) {
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

            y -= 60;
            if (textDeleted)
                return;
        }


        if (selectedType != SelectedType::None)
        {
            // UPDATED: Added font parameter to DrawText
            render.DrawText("PROPERTIES", 50, 400, 1.2f, { 1,1,1 }, "Marker");
            float y = 360;

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

                float circleX = 40 + channel * 140; // 0R, 140G, 280B
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
                float circleX = 40 + textColorChannel * 140;
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
        }

        render.DrawRect({ 150, 990 }, 140, 50, { 0.3f, 0.6f, 0.3f });
        render.DrawRect({ 330, 990 }, 140, 50, { 0.3f, 0.6f, 0.3f });
        // UPDATED: Added font parameter to DrawText
        render.DrawText("+Button", 100, 980, 0.5f, { 1,1,1 }, "Marker");
        render.DrawText("+Text", 300, 980, 0.5f, { 1,1,1 }, "Marker");

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
            newText.color = { 1, 1, 1 };

            newText.basePos = newText.pos;
            newText.baseScale = newText.scale;

            textboxes.push_back(newText);

            selectedType = SelectedType::TextBox;
            selectedIndex = (int)buttons.size() + (int)textboxes.size() - 1; // auto-select new
        }
    }
}

void GuiSystem::Shutdown() {
    buttons.clear();
    audio = nullptr;
}