// System.h
#pragma once

class ISystem {
public:
    virtual void Initialize() {}
    virtual void Update(float dt) = 0;
    virtual void Shutdown() {}
    virtual ~ISystem() {}
};
