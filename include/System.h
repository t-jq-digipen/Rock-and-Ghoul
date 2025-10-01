// System.h
#pragma once
#include <vector>
#include "Entity.h"   // or wherever Entity typedef is

class Registry; // forward declare

class ISystem {
public:
    virtual void Initialize() {}
    virtual void Update(float dt) = 0;
    virtual void Shutdown() {}
    virtual ~ISystem() {}
};
