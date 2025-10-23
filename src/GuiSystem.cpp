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

void GuiSystem::AddButton(const std::string& text, const Vec2& pos, const Vec2& size,
    const Colour3& color, std::function<void()> callback,
    AudioSystem::ButtonType soundType) {
    buttons.push_back({ text, pos, size, color, callback, false, false, soundType });
}

void GuiSystem::Update(float dt) {
    Vec2 mouse = Input::GetMousePosition();
    Render& r = Render::GetInstance();

    for (auto& btn : buttons) {
        bool hover = r.IsAreaTouched(mouse, btn.pos, btn.size.x, btn.size.y);


        // --- Draw button ---
        Colour3 drawColor = hover ?
            Colour3{ btn.color.r + 0.2f, btn.color.g + 0.2f, btn.color.b + 0.2f } :
            btn.color;

        r.DrawRect(btn.pos, btn.size.x, btn.size.y, drawColor);
        r.DrawText(btn.label, btn.pos.x - btn.size.x * 0.15f, btn.pos.y - 10.f, 1.2f, { 1, 1, 1 }, "Marker");

        // --- Click action ---
        if (hover && Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
            if (audio) audio->PlayButtonSound(btn.soundType);
            if (btn.onClick) btn.onClick();
        }
    }
}

void GuiSystem::Shutdown() {
    buttons.clear();
    audio = nullptr;
}