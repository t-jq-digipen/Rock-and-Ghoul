#pragma once
#include "Entity.h"
#include <functional>


struct BehaviourFCT {
    std::function<void(Entity)> Init;
    std::function<void(Entity, float)> Update;
    std::function<void(Entity)> End;
};