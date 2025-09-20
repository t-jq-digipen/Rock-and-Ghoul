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

        renderer.RenderText("Rock & Ghoul", (float)GLApp::width/2.8f, (float)GLApp::height/1.15f, 2.5f, glm::vec3(0.3, 0.7f, 0.9f)); //currently rgb only, can tune to add transparency(alpha) if need
        renderer.RenderText("<Team Avid Divas>", (float)GLApp::width / 16.f, (float)GLApp::height / 12.f, 1.0f, glm::vec3(1.f, 0.5f, 0.5f));

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
