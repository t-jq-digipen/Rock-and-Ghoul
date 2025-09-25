// System.h
#pragma once
#include <vector>
#include "Entity.h"   // or wherever Entity typedef is

class Registry; // forward declare

class ISystem {
protected:
    std::vector<Entity> entities;
    Registry* registry = nullptr;
public:
    virtual void Initialize() {}
    virtual void Update(float dt) = 0;
    virtual void Shutdown() {}
    virtual ~ISystem() {}
    void AddEntity(Entity e) { entities.push_back(e); }
    void SetRegistry(Registry* r) { registry = r; }
};
