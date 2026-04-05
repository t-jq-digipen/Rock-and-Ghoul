/* ========================================================
 * @file        Spells.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu 100%
 * @date        01 OCT 2025
 *
 * @brief
 *
 * Declares the SpellManager class, which manages a fixed set
 * of spell slots for the player. Provides functions to load
 * and unload spell textures and save,
 * clear, or assign spells dynamically to slots.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Render.h"

struct SpellSlot {
    GLuint iconTex{ 0 };
    bool isFilled{ false };
};

class SpellManager {
public:
    static const int MAX_SLOTS = 4;

    SpellManager();

    void LoadTextures();
    void Draw(Render& renderer);

    void SaveSpell(int slotIndex, GLuint spellTex);
    void ClearSpell(int slotIndex);
    GLuint barrierTex, shockwaveTex, aoeTex, healTex;
    void GiveSpell(GLuint tex);
    void CleanupTextures();

private:
    SpellSlot slots[MAX_SLOTS];
};
