#include "Registry.h"
#include "Component.h"
#include <iostream>

extern "C" {

    __declspec(dllexport) void CS_GetPosition(unsigned int entity, float* x, float* y)
    {
        try {
            TransformComponent& t = Registry::Get().Get_Component<TransformComponent>(entity);
            *x = t.pos.x;
            *y = t.pos.y;
        }
        catch (...) {
            *x = *y = 0.f;
            std::cerr << "[MonoBridge] Failed to get position for entity " << entity << "\n";
        }
    }

    __declspec(dllexport) void CS_SetPosition(unsigned int entity, float x, float y)
    {
        try {
            TransformComponent& t = Registry::Get().Get_Component<TransformComponent>(entity);
            t.pos.x = x;
            t.pos.y = y;
        }
        catch (...) {
            std::cerr << "[MonoBridge] Failed to set position for entity " << entity << "\n";
        }
    }

}
