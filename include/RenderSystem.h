/* ========================================================
 * @file        RenderSystem.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        01 OCT 2025
 *
 * @brief
 *
 * fella never even document this ------------- FIXTHISSHIT
 * Declares the RenderSystem, which is responsible for
 * drawing all visual components in the ECS framework.
 * -------------------------------------- end of FIXTHISSHIT
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
#include "ImguiSystem.h"
class RenderSystem : public ISystem {
public:
    /*Render& renderer;
    RenderSystem(Render& r) : renderer(r) {}*/
    RenderSystem() {}
    void Initialize() override;
    void Update(float dt) override;
    void Shutdown() override;
};
