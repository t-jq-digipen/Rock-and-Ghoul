#pragma once
#include <string>
#include <vector>
#include "Component.h"

// component to load from json file
struct ComponentData {
    ComponentTypeID type;
    std::string jsonData;  // serialized component data
};

// json entity struct
struct SavedEntity {
    int id;
    std::string name;
    std::string type;
    float x, y;
    int health;
    std::unordered_map<ComponentTypeID, std::string> components;
    SavedEntity() : id(0), name(""), type(""), x(0.0f), y(0.0f), health(100) {}
};

// json map struct
struct SavedMap {
    std::string name;
    int width;
    int height;
    std::vector<std::vector<int>> tiles;
    std::vector<SavedEntity> entities;

    SavedMap() : name(""), width(0), height(0) {}
};

// json game settings struct
struct GameSettings {
    float volume;
    int resolutionWidth;
    int resolutionHeight;
    bool fullscreen;

    GameSettings() : volume(1.0f), resolutionWidth(1920), resolutionHeight(1080), fullscreen(false) {}
};