#pragma once
#include "System.h"
#include "Render.h"
#include "Input.h"
#include "AudioSystem.h"
#include <functional>
#include <vector>
#include <string>

struct GUIButton {
    std::string label;
    Vec2 pos;
    Vec2 size;
    Colour3 color;
    std::function<void()> onClick;
    bool hovered = false;
    bool hoverPlayed = false;
    AudioSystem::ButtonType soundType = AudioSystem::ButtonType::PLAY;
};

class GuiSystem : public ISystem {
public:
    void Initialize() override;
    void Update(float dt) override;
    void Shutdown() override;

    void AddButton(const std::string& text, const Vec2& pos, const Vec2& size,
        const Colour3& color, std::function<void()> callback,
        AudioSystem::ButtonType soundType = AudioSystem::ButtonType::PLAY);

    static void SetAudioManager(AudioManager* audioMgr);

private:
    std::vector<GUIButton> buttons;
    static AudioManager* audio;
    Render& render = Render::GetInstance();
};
