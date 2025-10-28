#pragma once
#include "System.h"
#include "Render.h"
#include "Movement.h"

class MovementSystem : public ISystem {
public:
    MovementSystem() {}
    void Initialize() override;
    void Update(float dt) override;
    void Shutdown() override;
};