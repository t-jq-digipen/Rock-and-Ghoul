/* ========================================================
 * @file        RenderSystem.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        01 OCT 2025
 *
 * @brief
 *
 * Declares the RenderSystem, which handles rendering of all
 * visual components in the ECS framework. This includes drawing
 * sprites, shapes, and optional debug overlays, integrating with
 * OpenGL and ImGui for visualization.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include "System.h"
#include "Registry.h"
#include "Component.h"
#include "glapp.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Debug.h"
#include "Input.h"
#ifndef NO_IMGUI
#include "ImguiSystem.h"
#endif
class RenderSystem : public ISystem {
public:
    RenderSystem() {}
    void Initialize() override;
    void Update(float dt) override;
    void Shutdown() override;

    static void AddBackgroundTexture(GLuint texID, const std::string& path);
    static std::vector<std::string> GetBackgroundTexturePaths();
	static void SetCutscenePath(const std::string& path);
    static void ClearBackgroundTextures();
    static bool* GetLayerEnabledArray();

    static float pixelsPerUnit;
    static float unitsPerPixel;
private:
    Registry& m_registry = Registry::Get();
    static GLuint s_backgroundTexture; // Static background texture
};

namespace CamFollowScript {
    inline void Start(Entity eID);

    inline void Update(Entity eID, float dt);

    inline void End(Entity eID);
}