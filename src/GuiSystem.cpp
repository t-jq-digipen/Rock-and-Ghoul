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
}

void GuiSystem::AddButton(const ButtonConfig& cfg)
{
    buttons.push_back(cfg);
}

void GuiSystem::Update(float dt) {
    Vec2 mouse = Input::GetMousePosition();
    Render& r = Render::GetInstance();

    for (auto& cfg : buttons) {
        bool hover = false;
        if (cfg.shape == ButtonShape::Circle) {

            float radius = std::min(cfg.size.x, cfg.size.y) * 0.5f;
            float dist = RG_Distance( { mouse.x, static_cast<float>(GLApp::height) - mouse.y }, cfg.pos);
            hover = dist <= radius;
        }
        else {
            hover = r.IsAreaTouched(mouse, cfg.pos, cfg.size.x, cfg.size.y);
        }

        Colour3 drawColor = hover
            ? Colour3{ cfg.color.r + 0.2f, cfg.color.g + 0.2f, cfg.color.b + 0.2f }
        : cfg.color;

        switch (cfg.shape) {
            case ButtonShape::Rectangle:
                r.DrawRect(cfg.pos, cfg.size.x, cfg.size.y, drawColor);
                break;
            case ButtonShape::Circle: {
                float radius = std::min(cfg.size.x, cfg.size.y) * 0.5f;
                r.DrawCircle(cfg.pos, radius, drawColor, 48);
                break;
                }
            case ButtonShape::Rounded: {
                float corner = std::min(cfg.size.x, cfg.size.y) * 0.5f;
                r.DrawRoundedRect(cfg.pos, cfg.size.x, cfg.size.y, corner, drawColor);
                break;
                }
        }

        float textScale = 1.2f;
        Vec2 textSize = r.CalculateTextSize(cfg.label, textScale);

        // text horizontally and vertically on the button
        float textX = cfg.pos.x - textSize.x * 0.5f;
        float textY = cfg.pos.y - textSize.y * 0.5f;

        //small vertical offset
        textY += textSize.y * 0.15f;

        r.DrawText(cfg.label, textX, textY, textScale, { 0, 0, 0 });

        if (hover && Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
            if (audio) audio->PlayButtonSound(cfg.soundType);
            if (cfg.onClick) cfg.onClick();
        }
    }

}

void GuiSystem::Shutdown() {
    buttons.clear();
    audio = nullptr;
}