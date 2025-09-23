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

private:
    SpellSlot slots[MAX_SLOTS];
};
