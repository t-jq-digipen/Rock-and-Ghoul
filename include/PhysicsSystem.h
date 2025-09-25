// PhysicsSystem.h
#pragma once
#include "System.h"
#include "Render.h"

class PhysicsSystem : public ISystem {
    Render& renderer;
public:
    PhysicsSystem(Render& r) : renderer(r) {}
    void Initialize() override;
    void Update(float dt) override;
    void Shutdown() override;
};
