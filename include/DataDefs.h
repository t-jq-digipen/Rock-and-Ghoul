#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Component.h"

// Simple entity save structure - just ID and component data as strings
struct SavedEntity {
    int id;
    std::string name;
    std::unordered_map<ComponentTypeID, std::string> componentData;

    SavedEntity() : id(0), name("") {}
};

struct SavedPhysics {
    float velX, velY;
    float decay;
    bool willCollide;
    // AABB data (center + dim for Set_AABB)
    float aabbCenterX, aabbCenterY;
    float aabbDimX, aabbDimY;
    // Circle data
    float circlePosX, circlePosY;
    float circleRad;
};

// Scene/Level
struct SavedScene {
    std::string sceneName;
    std::vector<SavedEntity> entities;

    SavedScene() : sceneName("Level1") {}
};

// Game settings
struct GameSettings {
    float volume;
    int resolutionWidth;
    int resolutionHeight;
    bool fullscreen;

    GameSettings() : volume(1.0f), resolutionWidth(1920), resolutionHeight(1080), fullscreen(false) {}
};