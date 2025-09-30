#include "JsonLoad.h"
#include "FileIO.h"
#include "Registry.h"
#include "Render.h"
#include <iostream>

#pragma region ComponentSavers
std::string JsonLoad::Save_Transform(Entity eID) {
    auto& t = Get_TransformComponent(eID);
    json j = {
        {"posX", t.pos.x},
        {"posY", t.pos.y},
        {"scaleX", t.scale.x},
        {"scaleY", t.scale.y}
    };
    return j.dump();
}

//std::string JsonLoad::Save_Sprite(Entity eID) {
//    auto& s = Get_SpriteComponent(eID);
//
//    // Get texture path
//    std::string texPath = "";
//    if (s.textureID != 0) {
//        texPath = Render::GetInstance().GetTexturePath(s.textureID);
//    }
//
//    json j = {
//        {"texturePath", texPath},
//        {"sizeX", s.size.x},
//        {"sizeY", s.size.y},
//        {"colorX", s.color.x},
//        {"colorY", s.color.y},
//        {"colorZ", s.color.z},
//        { "shape", static_cast<int>(s.shape) }
//    };
//    return j.dump();
//}

std::string JsonLoad::Save_Sprite(Entity eID) {
    auto& s = Get_SpriteComponent(eID);

    // Get texture path from the image
    std::string texPath = s.Get_TexturePath();

    json j = {
        {"texturePath", texPath},
        {"scaleX", s.Get_Scale().x},
        {"scaleY", s.Get_Scale().y},
        {"animated", s.IsAnimated()}
    };
    return j.dump();
}

std::string JsonLoad::Save_Shape(Entity eID) {
    auto& s = Get_ShapeComponent(eID);
    json j = {
        {"shapeType", static_cast<int>(s.shape)},
        {"dimX", s.dimensions.x},
        {"dimY", s.dimensions.y},
        {"colorX", s.color.x},
        {"colorY", s.color.y},
        {"colorZ", s.color.z}
    };
    return j.dump();
}

std::string JsonLoad::Save_Physics(Entity eID) {
    auto& p = Get_PhysicsComponent(eID);
    auto vel = p.Get_Vel();
    auto aabb = p.Get_AABB();
    auto circle = p.Get_Circle();

    // Calculate center from AABB
    float centerX = (aabb.min.x + aabb.max.x) * 0.5f;
    float centerY = (aabb.min.y + aabb.max.y) * 0.5f;

    json j = {
        {"velX", vel.x},
        {"velY", vel.y},
        {"decay", 0.015f},
        {"willCollide", p.CollisionActive()},
        {"aabb", {
            {"centerX", centerX},
            {"centerY", centerY},
            {"dimX", aabb.dim.x},      // Save the dim field
            {"dimY", aabb.dim.y}       // Save the dim field
        }},
        {"circle", {
            {"posX", circle.pos.x},
            {"posY", circle.pos.y},
            {"rad", circle.rad}
        }}
    };
    return j.dump();
}

std::string JsonLoad::Save_Health(Entity eID) {
    auto& h = Get_HealthComponent(eID);
    json j = {
        {"currentHP", h.currhp},
        {"maxHP", h.maxhp}
    };
    return j.dump();
}

std::string JsonLoad::Save_Tag(Entity eID) {
    auto& t = Get_TagComponent(eID);
    json j = {
        {"tagID", static_cast<int>(t.tag)}
    };
    return j.dump();
}

std::string JsonLoad::Save_Input(Entity eID) {
    auto& i = Get_InputComponent(eID);
    json j = {
        {"scheme", static_cast<int>(i.scheme)}
    };
    return j.dump();
}
#pragma endregion ComponentSavers

#pragma region ComponentLoaders

void JsonLoad::Load_Transform(Entity eID, const std::string& data) {
    json j = json::parse(data);
    TransformComponent t;
    t.pos.x = j["posX"];
    t.pos.y = j["posY"];
    t.scale.x = j["scaleX"];
    t.scale.y = j["scaleY"];
    Add_Component(eID, t);
}
void JsonLoad::Load_Shape(Entity eID, const std::string& data) {
    json j = json::parse(data);
    ShapeComponent s;

    s.shape = static_cast<ShapeType>(static_cast<int>(j["shapeType"]));
    s.dimensions.x = j["dimX"];
    s.dimensions.y = j["dimY"];
    s.color.x = j["colorX"];
    s.color.y = j["colorY"];
    s.color.z = j["colorZ"];

    Add_Component(eID, s);
}

//void JsonLoad::Load_Sprite(Entity eID, const std::string& data) {
//    json j = json::parse(data);
//    SpriteComponent s;
//
//    // Reload texture from path
//    std::string texPath = j["texturePath"];
//    if (!texPath.empty()) {
//        s.textureID = Render::GetInstance().LoadTexture(texPath);
//    }
//    else {
//        s.textureID = 0;
//    }
//
//    s.size.x = j["sizeX"];
//    s.size.y = j["sizeY"];
//    s.color.x = j["colorX"];
//    s.color.y = j["colorY"];
//    s.color.z = j["colorZ"];
//    if (j.contains("shape")) {
//        s.shape = static_cast<ShapeType>(static_cast<int>(j["shape"]));
//    }
//    else {
//        s.shape = (s.textureID == 0) ? SHAPE_RECT : SHAPE_TEXTURED;
//    }
//    Add_Component(eID, s);
//}
void JsonLoad::Load_Sprite(Entity eID, const std::string& data) {
    json j = json::parse(data);

    std::string texPath = j["texturePath"];
    Vec2 scale;
    scale.x = j["scaleX"];
    scale.y = j["scaleY"];
    bool animated = j["animated"];

    // Create sprite component with the loaded data
    if (!texPath.empty()) {
        Sprite::SpriteComponent s(texPath, scale, animated);
        Add_Component(eID, s);
    }
}

void JsonLoad::Load_Physics(Entity eID, const std::string& data) {
    json j = json::parse(data);

    // Build AABB using Set_AABB helper (which sets dim automatically)
    Vec2 center = { j["aabb"]["centerX"], j["aabb"]["centerY"] };
    Vec2 size = { j["aabb"]["dimX"], j["aabb"]["dimY"] };
    AABB aabb = Set_AABB(center, size);

    // Build Circle
    Circle circle;
    circle.pos.x = j["circle"]["posX"];
    circle.pos.y = j["circle"]["posY"];
    circle.rad = j["circle"]["rad"];

    // Create Physics Component
    float decay = j["decay"];
    bool willCollide = j["willCollide"];
    Physics::PhysicsComponent p(decay, willCollide, aabb, circle);

    // Set velocity
    Vec2 vel;
    vel.x = j["velX"];
    vel.y = j["velY"];
    p.Set_Vel(vel);

    Add_Component(eID, p);
}

void JsonLoad::Load_Health(Entity eID, const std::string& data) {
    json j = json::parse(data);
    HealthComponent h;
    h.currhp = j["currentHP"];
    h.maxhp = j["maxHP"];
    Add_Component(eID, h);
}

void JsonLoad::Load_Tag(Entity eID, const std::string& data) {
    json j = json::parse(data);
    TagComponent t;
    t.tag = static_cast<C_TagID>(static_cast<int>(j["tagID"]));
    Add_Component(eID, t);
}

void JsonLoad::Load_Input(Entity eID, const std::string& data) {
    json j = json::parse(data);
    InputComponent i;
    i.scheme = static_cast<ControlScheme>(static_cast<int>(j["scheme"]));
    Add_Component(eID, i);
}
#pragma endregion ComponentSavers

// ========================================
// ECS <-> SAVED ENTITY CONVERSION
// ========================================

SavedEntity JsonLoad::EntityToSaved(Entity eID) {
    SavedEntity saved;
    saved.id = eID;
    saved.name = GetEntityName(eID);

    // Check each component and save if exists
    if (Has_Component(eID, CT_Transform)) {
        saved.componentData[CT_Transform] = Save_Transform(eID);
    }

    if (Has_Component(eID, CT_Sprite)) {
        saved.componentData[CT_Sprite] = Save_Sprite(eID);
    }

    if (Has_Component(eID, CT_Physics)) {
        saved.componentData[CT_Physics] = Save_Physics(eID);
    }

    if (Has_Component(eID, CT_Health)) {
        saved.componentData[CT_Health] = Save_Health(eID);
    }

    if (Has_Component(eID, CT_Tag)) {
        saved.componentData[CT_Tag] = Save_Tag(eID);
    }
    if (Has_Component(eID, CT_Shape)) {  // ADD THIS
        saved.componentData[CT_Shape] = Save_Shape(eID);
    }

    if (Has_Component(eID, CT_Input)) {
        saved.componentData[CT_Input] = Save_Input(eID);
    }

    return saved;
}

Entity JsonLoad::SavedToEntity(const SavedEntity& saved) {
    Entity e = Create_Entity();
    SetEntityName(e, saved.name);  // get the name of component stored

    std::cout << "Creating entity '" << saved.name << "' (ID: " << e << ")" << std::endl;

    // Loop through component data and load each component
    for (const auto& [componentType, data] : saved.componentData) {
        switch (componentType) {
        case CT_Transform:
            Load_Transform(e, data);
            break;

       case CT_Sprite:
            Load_Sprite(e, data);
            break;

        case CT_Physics:
            Load_Physics(e, data);
            break;

        case CT_Health:
            Load_Health(e, data);
            break;

        case CT_Tag:
            Load_Tag(e, data);
            break;

        case CT_Input:
            Load_Input(e, data);
            break;
        case CT_Shape:  // ADD THIS CASE
            Load_Shape(e, data);
            break;

        default:
            break;
        }
    }

    return e;
}

#pragma region Serialization

void JsonLoad::to_json(json& j, const SavedEntity& e) {
    j = json{
        {"id", e.id},
        {"name", e.name},
        {"components", json::object()}
    };

    // Add component data
    for (const auto& [componentType, data] : e.componentData) {
        std::string componentName;
        switch (componentType) {
        case CT_Transform: componentName = "Transform"; break;
        case CT_Sprite: componentName = "Sprite"; break;
        case CT_Physics: componentName = "Physics"; break;
        case CT_Health: componentName = "Health"; break;
        case CT_Tag: componentName = "Tag"; break;
        case CT_Input: componentName = "Input"; break;
        case CT_Shape: componentName = "Shape"; break;
        default: componentName = "Unknown"; break;
        }

        // Parse the JSON string and add it
        j["components"][componentName] = json::parse(data);
    }
}

void JsonLoad::from_json(const json& j, SavedEntity& e) {
    e.id = j["id"];
    e.name = j["name"];

    // Read component data
    const json& components = j["components"];

    for (auto it = components.begin(); it != components.end(); ++it) {
        const std::string& componentName = it.key();
        ComponentTypeID componentType = CT_None;

        if (componentName == "Transform") componentType = CT_Transform;
        else if (componentName == "Sprite") componentType = CT_Sprite;
        else if (componentName == "Physics") componentType = CT_Physics;
        else if (componentName == "Health") componentType = CT_Health;
        else if (componentName == "Tag") componentType = CT_Tag;
        else if (componentName == "Shape") componentType = CT_Shape;
        else if (componentName == "Input") componentType = CT_Input;

        if (componentType != CT_None) {
            e.componentData[componentType] = it.value().dump();
        }
    }
}

void JsonLoad::to_json(json& j, const SavedScene& s) {
    j = json{
        {"sceneName", s.sceneName},
        {"entities", json::array()}
    };

    for (const auto& entity : s.entities) {
        json entityJson;
        to_json(entityJson, entity);
        j["entities"].push_back(entityJson);
    }
}

void JsonLoad::from_json(const json& j, SavedScene& s) {
    s.sceneName = j["sceneName"];

    s.entities.clear();
    for (const auto& entityJson : j["entities"]) {
        SavedEntity entity;
        from_json(entityJson, entity);
        s.entities.push_back(entity);
    }
}

void JsonLoad::to_json(json& j, const GameSettings& s) {
    j = json{
        {"volume", s.volume},
        {"resolutionWidth", s.resolutionWidth},
        {"resolutionHeight", s.resolutionHeight},
        {"fullscreen", s.fullscreen}
    };
}

void JsonLoad::from_json(const json& j, GameSettings& s) {
    j.at("volume").get_to(s.volume);
    j.at("resolutionWidth").get_to(s.resolutionWidth);
    j.at("resolutionHeight").get_to(s.resolutionHeight);
    j.at("fullscreen").get_to(s.fullscreen);
}
#pragma region Serialization

// ========================================
// HIGH-LEVEL SAVE/LOAD FUNCTIONS
// ========================================

bool JsonLoad::SaveCurrentScene(const std::string& filepath, const std::string& sceneName) {
    try {
        SavedScene scene;
        scene.sceneName = sceneName;

        // Get all alive entities
        EntityManager& em = EntityManager::get();
        int entityCount = 0;
        for (Entity e = 0; e < em.maxEntities(); ++e) {
            if (em.isAlive(e)) {
                std::cout << "Saving entity ID: " << e << std::endl;

                // Debug: check what components it has
                if (Has_Component(e, CT_Transform)) std::cout << "  - Has Transform" << std::endl;
                if (Has_Component(e, CT_Sprite)) std::cout << "  - Has Sprite" << std::endl;
                if (Has_Component(e, CT_Physics)) std::cout << "  - Has Physics" << std::endl;
                if (Has_Component(e, CT_Health)) std::cout << "  - Has Health" << std::endl;
                if (Has_Component(e, CT_Tag)) std::cout << "  - Has Tag" << std::endl;
                if (Has_Component(e, CT_Input)) std::cout << "  - Has Input" << std::endl;

                SavedEntity saved = EntityToSaved(e);
                scene.entities.push_back(saved);
                entityCount++;
            }
        }

        std::cout << "Total entities saved: " << entityCount << std::endl;

        json j;
        to_json(j, scene);
        return FileIO::WriteFile(filepath, j.dump(4));
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving scene: " << e.what() << std::endl;
        return false;
    }
}

bool JsonLoad::LoadScene(const std::string& filepath) {
    try {
        std::string content = FileIO::ReadFile(filepath);
        if (content.empty()) {
            std::cerr << "File is empty or doesn't exist: " << filepath << std::endl;
            return false;
        }

        json j = json::parse(content);
        SavedScene scene;
        from_json(j, scene);

        std::cout << "Loading scene: " << scene.sceneName << std::endl;
        std::cout << "Number of entities: " << scene.entities.size() << std::endl;

        // Create all entities
        for (const auto& savedEntity : scene.entities) {
            Entity e = SavedToEntity(savedEntity);
            std::cout << "Created entity ID: " << e << " (was: " << savedEntity.id << ")" << std::endl;
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading scene: " << e.what() << std::endl;
        return false;
    }
}

bool JsonLoad::SaveSettings(const std::string& filepath, const GameSettings& settings) {
    try {
        json j;
        to_json(j, settings);
        return FileIO::WriteFile(filepath, j.dump(4));
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving settings: " << e.what() << std::endl;
        return false;
    }
}

std::optional<GameSettings> JsonLoad::LoadSettings(const std::string& filepath) {
    try {
        std::string content = FileIO::ReadFile(filepath);
        if (content.empty()) return std::nullopt;

        json j = json::parse(content);
        GameSettings settings;
        from_json(j, settings);
        return settings;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading settings: " << e.what() << std::endl;
        return std::nullopt;
    }
}
// Clear all entities before loading
void JsonLoad::ClearAllEntities() {
    EntityManager& em = EntityManager::get();
    std::vector<Entity> toRemove;

    // Collect all alive entities
    for (Entity e = 0; e < em.maxEntities(); ++e) {
        if (em.isAlive(e)) {
            toRemove.push_back(e);
        }
    }

    // Remove them
    for (Entity e : toRemove) {
        Remove_Entity(e);
    }

    std::cout << "Cleared " << toRemove.size() << " entities" << std::endl;
}