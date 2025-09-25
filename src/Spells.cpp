#include "Spells.h"
#include "Remath.h"
#include <iostream>
SpellManager::SpellManager() {
    std::cout << "SpellManager constructed: " << this << "\n";
    for (int i = 0; i < MAX_SLOTS; i++) {
        slots[i].isFilled = false;
        slots[i].iconTex = 0;
    }
}

void SpellManager::LoadTextures() {
    barrierTex = Render::GetInstance().LoadTexture("../Assets/barrier.png");
    shockwaveTex = Render::GetInstance().LoadTexture("../Assets/shockwave.png");
    aoeTex = Render::GetInstance().LoadTexture("../Assets/aoe.png");
    healTex = Render::GetInstance().LoadTexture("../Assets/heal.png");
}

void SpellManager::Draw(Render& renderer) {
    //std::cout << "Drawing with SpellManager: " << this << "\n";
    float slotSize = 100.0f;
    float space = 10.0f;

    // Assuming 1280x720 resolution, bottom right corner
    float baseX = 1280.0f - (slotSize + space);
    float baseY = 60.0f;

    for (int i = 0; i < MAX_SLOTS; i++) {
        Vec2 pos = { baseX - i * (slotSize + space), baseY };
        Vec2 size = { slotSize, slotSize };

        Colour3 grey = { 0.2f, 0.2f, 0.2f };
        renderer.DrawRect(pos, size.x, size.y, grey);

        if (slots[i].isFilled && slots[i].iconTex != 0) {
            renderer.DrawTexturedQuad(pos, size, slots[i].iconTex);
        }
    }
}

void SpellManager::SaveSpell(int slotIndex, GLuint spellTex) {
    if (slotIndex >= 0 && slotIndex < MAX_SLOTS) {
        slots[slotIndex].iconTex = spellTex;
        slots[slotIndex].isFilled = true;
    }
}

void SpellManager::ClearSpell(int slotIndex) {
    if (slotIndex >= 0 && slotIndex < MAX_SLOTS) {
        slots[slotIndex].iconTex = 0;
        slots[slotIndex].isFilled = false;
    }
}

void SpellManager::GiveSpell(GLuint tex) {
    std::cout << "GiveSpell CALLED!\n";
    for (int i = 0; i < MAX_SLOTS; i++) {
        if (!slots[i].isFilled) {
            SaveSpell(i, tex);
            std::cout << "GiveSpell: tex = " << tex << "\n";
            break;
        }
    }
}
