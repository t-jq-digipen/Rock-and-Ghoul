/* ========================================================
 * @file        Spells.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        01 OCT 2025
 *
 * @brief
 *
 * fella never even document this ------------- FIXTHISSHIT
 * Declares the SpellManager class which does spellmanager
 * things
 * -------------------------------------- end of FIXTHISSHIT
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
