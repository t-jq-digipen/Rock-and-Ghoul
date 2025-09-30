// RenderSystem.h
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
