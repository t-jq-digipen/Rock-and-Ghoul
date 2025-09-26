// PhysicsSystem.h
#pragma once
#include "System.h"
#include "Render.h"

class PhysicsSystem : public ISystem {
public:
    PhysicsSystem() {}
    void Initialize() override;
    void Update(float dt) override;
    void Shutdown() override;
};
