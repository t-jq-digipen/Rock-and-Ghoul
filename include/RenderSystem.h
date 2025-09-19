// RenderSystem.h
#pragma once
#include "System.h"
#include "Render.h"

class RenderSystem : public ISystem {
    Render renderer;
    GLuint fretboardTexture{ 0 };
public:
    void Initialize() override {
        renderer.Init();
        fretboardTexture = Render::LoadTexture("../Assets/fretboard.jpg");
    }

    void Update(float) override {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        if (fretboardTexture) {
            renderer.DrawTexturedQuad({ 1200.0f, 200.0f }, { 1000.0f, 200.0f }, fretboardTexture);
            /*int bgW = 0, bgH = 0;
            glBindTexture(GL_TEXTURE_2D, fretboardTexture);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &bgW);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &bgH);
            renderer.DrawBackground(fretboardTexture, bgW, bgH, BgFit::Cover);*/
        }
    }

    void Shutdown() override {
        if (fretboardTexture) {
            glDeleteTextures(1, &fretboardTexture);
            fretboardTexture = 0;
        }
    }

    Render& GetRenderer() { return renderer; }
};
