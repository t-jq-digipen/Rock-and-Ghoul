/* ========================================================
 * @file        JsonLoad.cpp
 * @author      Yasmin Tazi | yasmin.t@digipen.edu
 * @co-author   Tan Ying Clarissa | t.yingclarissa@digipen.edu
 * @date        26 SEP 2025
 *
 * @brief   Implementation file for JSON serialization system
 *
 * This file contains the implementation of the JsonLoad system,
 * which provides JSON serialization and deserialization
 * functionality for the game's Entity Component System (ECS). The
 * system enables saving and loading of individual components, complete
 * entities, entire game scenes, and game settings. It uses the nlohmann
 * JSON library for parsing and serialization, and provides type-safe
 * conversion between game objects and their JSON representations.
 *
 * Copyright � 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include "JsonLoad.h"
#include "FileIO.h"
#include "Registry.h"
#include "Render.h"
#include <iostream>

#pragma region ComponentSavers

 /**
  * @brief Serializes Transform component to JSON string.
  *
  * @param eID The entity ID whose Transform component should be serialized.
  * @return std::string JSON string containing the transform data with fields:
  *         posX, posY, scaleX, scaleY, and rot.
  */
//std::string JsonLoad::Save_Transform(Entity eID) {
//    auto& t = Registry::Get().Get_Component<TransformComponent>(eID);
//    json j = {
//        {"posX", t.pos.x},
//        {"posY", t.pos.y},
//        {"scaleX", t.scale.x},
//        {"scaleY", t.scale.y},
//        {"rot", t.rot}
//    };
//    return j.dump();
//}

/**
 * @brief Serializes Sprite component to JSON string.
 *
 * @param eID The entity ID whose Sprite component should be serialized.
 * @return std::string JSON string containing the sprite data with fields:
 *         texturePath, scaleX, scaleY, and animated.
 */
//std::string JsonLoad::Save_Sprite(Entity eID) {
//    auto& s = Registry::Get().Get_Component<Sprite::SpriteComponent>(eID);
//
//    // Get texture path from the image
//    std::string texPath = s.Get_TexturePath();
//
//    json j = {
//        {"texturePath", texPath},
//        {"scaleX", s.Get_Scale().x},
//        {"scaleY", s.Get_Scale().y},
//        {"animated", s.IsAnimated()}
//
//    };
//    return j.dump();
//}

/**
 * @brief Serializes Animator component to JSON string.
 *
 * @param eID The entity ID whose Animator component should be serialized.
 * @return std::string JSON string containing the animator data including:
 *         totalRows, totalCols, animDuration, frame configuration,
 *         currentState, all animation states, and UV coordinates (u0, v0, u1, v1).
 */
//std::string JsonLoad::Save_Animator(Entity eID) {
//    auto& a = Registry::Get().Get_Component<Animator::AnimatorComponent>(eID);
//
//    auto frames = a.GetFrames();
//    auto stateMap = a.GetStateFrames();
//
//    json statesJson = json::object();
//    for (const auto& [state, stateFrames] : stateMap) {
//        statesJson[std::to_string(state)] = { {"row", stateFrames.row}, {"col_start", stateFrames.col_start}, {"col_end", stateFrames.col_end}, {"duration", stateFrames.duration} };
//    }
//
//    json j = {
//        {"totalRows", a.GetTotalRows()},
//        {"totalCols", a.GetTotalCols()},
//        {"animDuration", a.GetAnimDuration()},
//        {"frameRow", frames.row},
//        {"frameColStart", frames.col_start},
//        {"frameColEnd", frames.col_end},
//        {"currentState", a.GetCurrentState()},
//        {"states", statesJson},
//        {"u0", a.u0},
//        {"v0", a.v0},
//        {"u1", a.u1},
//        {"v1", a.v1}
//    };
//    return j.dump();
//}

/**
 * @brief Serializes Shape component to JSON string.
 *
 * @param eID Entity ID Shape component should be serialized.
 * @return std::string JSON string containing the shape data
 */
//std::string JsonLoad::Save_Shape(Entity eID) {
//    auto& s = Registry::Get().Get_Component<ShapeComponent>(eID);
//    json j = {
//        {"shapeType", static_cast<int>(s.shape)},
//        {"dimX", s.dimensions.x},
//        {"dimY", s.dimensions.y},
//        {"colorX", s.color.x},
//        {"colorY", s.color.y},
//        {"colorZ", s.color.z}
//    };
//    return j.dump();
//}

/**
 * @brief Serializes Physics component to JSON string.
 *
 * @param eID The entity ID whose Physics component should be serialized.
 * @return std::string JSON string containing the physics data.
 */
//std::string JsonLoad::Save_Physics(Entity eID) {
//    auto& p = Registry::Get().Get_Component<Physics::PhysicsComponent>(eID);
//    auto& vel = p.Get_Vel();
//    auto& aabb = p.Get_AABB();
//    auto& circle = p.Get_Circle();
//
//    // Calculate center from AABB
//    float centerX = (aabb.min.x + aabb.max.x) * 0.5f;
//    float centerY = (aabb.min.y + aabb.max.y) * 0.5f;
//
//    json j = {
//        {"velX", vel.x},
//        {"velY", vel.y},
//        {"decay", 0.015f},
//        {"willCollide", p.CollisionActive()},
//        {"aabb", {
//            {"centerX", centerX},
//            {"centerY", centerY},
//            {"dimX", aabb.dim.x},      // Save the dim field
//            {"dimY", aabb.dim.y}       // Save the dim field
//        }},
//        {"circle", {
//            {"posX", circle.pos.x},
//            {"posY", circle.pos.y},
//            {"rad", circle.rad}
//        }}
//    };
//    return j.dump();
//}

/**
 * @brief Serializes Health component to JSON string.
 *
 * @param eID The entity ID where Health component should be serialized.
 * @return std::string JSON string containing the health data.
 */
//std::string JsonLoad::Save_Health(Entity eID) {
//    auto& h = Registry::Get().Get_Component<HealthComponent>(eID);
//    json j = {
//        {"currentHP", h.currhp},
//        {"maxHP", h.maxhp}
//    };
//    return j.dump();
//}

/**
 * @brief Serializes Tag component to JSON string.
 *
 * @param eID The entity ID whose Tag component should be serialized.
 * @return std::string JSON string containing the tag data.
 */
//std::string JsonLoad::Save_Tag(Entity eID) {
//    auto& t = Registry::Get().Get_Component<TagComponent>(eID);
//    json j = {
//        {"tagID", static_cast<int>(t.tag)}
//    };
//    return j.dump();
//}

/**
 * @brief Serializes Input component to JSON string.
 *
 * @param eID The entity ID where Input component should be serialized.
 * @return std::string JSON string containing the input data.
 */
//std::string JsonLoad::Save_Input(Entity eID) {
//    auto& i = Registry::Get().Get_Component<InputComponent>(eID);
//    json j = {
//        {"scheme", static_cast<int>(i.scheme)}
//    };
//    return j.dump();
//}
#pragma endregion ComponentSavers

#pragma region ComponentLoaders

/**
 * @brief Deserializes and loads Transform component onto entity.
 *
 * @param eID The entity ID to which the Transform component should be added.
 * @param data JSON string containing transform data.
 */
//void JsonLoad::Load_Transform(Entity eID, const std::string& data) {
//    json j = json::parse(data);
//    TransformComponent t;
//    t.pos.x = j["posX"];
//    t.pos.y = j["posY"];
//    t.scale.x = j["scaleX"];
//    t.scale.y = j["scaleY"];
//    t.rot = j.value("rot", 0.0f);
//    Registry::Get().Add_Component<TransformComponent>(eID, t);
//}

/**
 * @brief Deserializes and loads Shape component onto entity.
 *
 * @param eID The entity ID to which the Shape component should be added.
 * @param data JSON string containing shape data.
 */
//void JsonLoad::Load_Shape(Entity eID, const std::string& data) {
//    json j = json::parse(data);
//    ShapeComponent s;
//
//    s.shape = static_cast<ShapeType>(static_cast<int>(j["shapeType"]));
//    s.dimensions.x = j["dimX"];
//    s.dimensions.y = j["dimY"];
//    s.color.x = j["colorX"];
//    s.color.y = j["colorY"];
//    s.color.z = j["colorZ"];
//
//    Registry::Get().Add_Component<ShapeComponent>(eID, s);
//}

/**
 * @brief Deserializes and loads Animator component onto entity.
 *
 * @param eID The entity ID to which the Animator component should be added.
 * @param data JSON string containing animator data.
 * @throws std::runtime_error If the entity doesn't have a Sprite component.
 */
//void JsonLoad::Load_Animator(Entity eID, const std::string& data) {
//    json j = json::parse(data);
//
//    // Reconstruct ActiveFrames
//    Animator::ActiveFrames frames;
//    frames.row = j["frameRow"];
//    frames.col_start = j["frameColStart"];
//    frames.col_end = j["frameColEnd"];
//    frames.duration = j.value("frameDuration", 1.0f);
//
//    if (!Registry::Get().Has_Component<Sprite::SpriteComponent>(eID)) {
//        std::cerr << "ERROR: Entity " << eID << " doesn't have Sprite component yet!" << std::endl;
//        throw std::runtime_error("Animator requires Sprite component");
//    }
//
//    // Get the sprite component to determine image dimensions
//    auto& sprite = Registry::Get().Get_Component<Sprite::SpriteComponent>(eID);
//    int imageWidth = sprite.Get_Image().widthPixels;
//    int imageHeight = sprite.Get_Image().heightPixels;
//
//    // Create animator with saved parameters
//    Animator::AnimatorComponent anim(j["totalRows"], j["totalCols"], imageWidth, imageHeight, frames, j.value("animDuration", 1.0f));
//
//    if (j.contains("states")) {
//        for (auto& [stateKey, stateData] : j["states"].items()) {
//            int stateID = std::stoi(stateKey);
//            Animator::ActiveFrames stateFrames;
//            stateFrames.row = stateData["row"];
//            stateFrames.col_start = stateData["col_start"];
//            stateFrames.col_end = stateData["col_end"];
//            stateFrames.duration = stateData.value("duration", 1.0f);
//
//            anim.AddState(stateID, stateFrames);
//        }
//    }
//
//    if (j.contains("currentState") && j["currentState"] != -1) {
//        anim.SetState(j["currentState"]);
//    }
//
//    Registry::Get().Add_Component<Animator::AnimatorComponent>(eID, anim);
//}

/**
 * @brief Deserializes and loads Sprite component onto entity.
 *
 * @param eID The entity ID to which the Sprite component should be added.
 * @param data JSON string containing sprite data (texturePath, scale, animated).
 */
//void JsonLoad::Load_Sprite(Entity eID, const std::string& data) {
//    json j = json::parse(data);
//
//    std::string texPath = j["texturePath"];
//    Vec2 scale;
//    scale.x = j["scaleX"];
//    scale.y = j["scaleY"];
//    bool animated = j["animated"];
//
//    if (!texPath.empty()) {
//        Sprite::SpriteComponent s(texPath, scale, animated);
//        Registry::Get().Add_Component<Sprite::SpriteComponent>(eID, s);
//    }
//}

/**
 * @brief Deserializes and loads Physics component onto entity.
 *
 * @param eID The entity ID to which the Physics component should be added.
 * @param data JSON string containing physics data.
 */
//void JsonLoad::Load_Physics(Entity eID, const std::string& data) {
//    json j = json::parse(data);
//
//    // Build AABB using Create_AABB helper
//    Vec2 center = { j["aabb"]["centerX"], j["aabb"]["centerY"] };
//    Vec2 size = { j["aabb"]["dimX"], j["aabb"]["dimY"] };
//    AABB aabb = Create_AABB(center, size);
//
//    // Build Circle
//    Circle circle;
//    circle.pos.x = j["circle"]["posX"];
//    circle.pos.y = j["circle"]["posY"];
//    circle.rad = j["circle"]["rad"];
//
//    // Create Physics Component
//    float decay = j["decay"];
//    bool willCollide = j["willCollide"];
//    Physics::PhysicsComponent p(decay, willCollide, aabb, circle);
//
//    // Set velocity
//    Vec2 vel;
//    vel.x = j["velX"];
//    vel.y = j["velY"];
//    p.Set_Vel(vel);
//
//    Registry::Get().Add_Component<Physics::PhysicsComponent>(eID, p);
//}

/**
 * @brief Deserializes and loads Health component onto entity.
 *
 * @param eID The entity ID to which the Health component should be added.
 * @param data JSON string containing health data.
 */
//void JsonLoad::Load_Health(Entity eID, const std::string& data) {
//    json j = json::parse(data);
//    HealthComponent h;
//    h.currhp = j["currentHP"];
//    h.maxhp = j["maxHP"];
//    Registry::Get().Add_Component<HealthComponent>(eID, h);
//}

/**
 * @brief Deserializes and loads Tag component onto entity.
 *
 * @param eID The entity ID to which the Tag component should be added.
 * @param data JSON string containing tag data.
 */
//void JsonLoad::Load_Tag(Entity eID, const std::string& data) {
//    json j = json::parse(data);
//    TagComponent t;
//    t.tag = static_cast<C_TagID>(static_cast<int>(j["tagID"]));
//    Registry::Get().Add_Component<TagComponent>(eID, t);
//}

/**
 * @brief Deserializes and loads Input component onto entity.
 *
 * @param eID The entity ID to which the Input component should be added.
 * @param data JSON string containing input data.
 */
//void JsonLoad::Load_Input(Entity eID, const std::string& data) {
//    json j = json::parse(data);
//    InputComponent i;
//    i.scheme = static_cast<ControlScheme>(static_cast<int>(j["scheme"]));
//    Registry::Get().Add_Component<InputComponent>(eID, i);
//}
#pragma endregion ComponentLoaders

// ECS <-> SAVED ENTITY CONVERSION

/**
 * @brief Converts live entity to SavedEntity structure.
 *
 * @param eID The entity ID to convert to a SavedEntity.
 * @return SavedEntity Structure containing the entity's ID, name, and all
 *         component data as JSON strings indexed by component type.
 */
//SavedEntity JsonLoad::EntityToSaved(Entity eID) {
//    SavedEntity saved;
//    saved.name = Registry::Get().Get_EntityName(eID);
//
//    // Check each component and save if exists
//    /*if (Registry::Get().Has_Component<TransformComponent>(eID)) {
//        saved.componentData[CT_Transform] = Save_Transform(eID);
//    }*/
//
//    /*if (Registry::Get().Has_Component<Sprite::SpriteComponent>(eID)) {
//        saved.componentData[CT_Sprite] = Save_Sprite(eID);
//    }*/
//
//    /*if (Registry::Get().Has_Component<Physics::PhysicsComponent>(eID)) {
//        saved.componentData[CT_Physics] = Save_Physics(eID);
//    }*/
//
//    /*if (Registry::Get().Has_Component<HealthComponent>(eID)) {
//        saved.componentData[CT_Health] = Save_Health(eID);
//    }*/
//
//    /*if (Registry::Get().Has_Component<TagComponent>(eID)) {
//        saved.componentData[CT_Tag] = Save_Tag(eID);
//    }
//
//    if (Registry::Get().Has_Component<ShapeComponent>(eID)) {
//        saved.componentData[CT_Shape] = Save_Shape(eID);
//    }
//
//    if (Registry::Get().Has_Component<InputComponent>(eID)) {
//        saved.componentData[CT_Input] = Save_Input(eID);
//    }
//
//    if (Registry::Get().Has_Component<Animator::AnimatorComponent>(eID)) {
//        saved.componentData[CT_Animator] = Save_Animator(eID);
//    }*/
//
//    return saved;
//}

/**
 * @brief Creates live entity from SavedEntity structure.
 *
 * @param saved The SavedEntity structure containing serialized component data.
 * @return Entity The ID of the newly created entity with all components attached.
 */
//Entity JsonLoad::SavedToEntity(const SavedEntity& saved) {
//    Entity e = Registry::Get().Create_Entity();
//    Registry::Get().Set_EntityName(e, saved.name);
//
//    std::cout << "Creating entity '" << saved.name << "' (ID: " << e << ")" << std::endl;
//
//    // Load Transform FIRST
//    /*auto transformIt = saved.componentData.find(CT_Transform);
//    if (transformIt != saved.componentData.end()) {
//        Load_Transform(e, transformIt->second);
//        std::cout << " Transform loaded" << std::endl;
//    }*/
//
//    // Load Sprite SECOND (before Animator needs it)
//    /*auto spriteIt = saved.componentData.find(CT_Sprite);
//    if (spriteIt != saved.componentData.end()) {
//        Load_Sprite(e, spriteIt->second);
//        std::cout << "Loaded Sprite" << std::endl;
//    }*/
//
//    // Then load other components
//    /*for (const auto& [componentType, data] : saved.componentData) {
//        if (componentType == CT_Transform || componentType == CT_Sprite) continue;*/
//
//        //try {
//        //    switch (componentType) {
//        //    case CT_Animator:
//        //        Load_Animator(e, data);
//        //        std::cout << "Loaded Animator" << std::endl;
//        //        break;
//        //    /*case CT_Physics:
//        //        Load_Physics(e, data);
//        //        std::cout << "Loaded Physics" << std::endl;
//        //        break;*/
//        //    /*case CT_Health:
//        //        Load_Health(e, data);
//        //        std::cout << "Loaded Health" << std::endl;
//        //        break;*/
//        //    case CT_Tag:
//        //        Load_Tag(e, data);
//        //        std::cout << "Loaded Tag" << std::endl;
//        //        break;
//        //    case CT_Shape:
//        //        Load_Shape(e, data);
//        //        std::cout << "Loaded Shape" << std::endl;
//        //        break;
//        //    case CT_Input:
//        //        Load_Input(e, data);
//        //        std::cout << "Loaded Input" << std::endl;
//        //        break;
//        //    default:
//        //        break;
//        //    }
//        //}
//        //catch (const std::exception& ex) {
//        //    std::cerr << "Failed to load component: " << ex.what() << std::endl;
//        //}
//    }
//
//    return e;
//}

#pragma region Serialization

/**
 * @brief Serializes SavedEntity to JSON object.
 *
 * @param j Reference to the JSON object to populate with entity data.
 * @param e SavedEntity to serialize.
 */
void JsonLoad::to_json(json& j, const SavedEntity& e) {
    j = json{
        // {"id", e.id},
        {"name", e.name},
        {"components", json::object()}
    };

    // add component data in a nice format
    for (const auto& [componentName, data] : e.componentData) {
        /*std::string componentName;
        switch (componentType) {
        case CT_Transform: componentName = "Transform"; break;
        case CT_Sprite: componentName = "Sprite"; break;
        case CT_Physics: componentName = "Physics"; break;
        case CT_Health: componentName = "Health"; break;
        case CT_Tag: componentName = "Tag"; break;
        case CT_Input: componentName = "Input"; break;
        case CT_Shape: componentName = "Shape"; break;
        case CT_Animator: componentName = "Animator"; break;
        default: componentName = "Unknown"; break;
        }*/

        // parse JSON string and add component data
        j["components"][componentName] = json::parse(data);
    }
}

/**
 * @brief Deserializes SavedEntity from JSON object.
 *
 * @param j The JSON object containing entity data.
 * @param e Reference to the SavedEntity to populate using deserialized data.
 */
void JsonLoad::from_json(const json& j, SavedEntity& e) {
    // e.id = j["id"];
    e.name = j["name"];

    // Read component data
    const json& components = j["components"];

    for (auto it = components.begin(); it != components.end(); ++it) {
        const std::string& componentName = it.key();
        //ComponentTypeID componentType = CT_None;

        /*if (componentName == "Transform") componentType = CT_Transform;
        else if (componentName == "Sprite") componentType = CT_Sprite;
        else if (componentName == "Physics") componentType = CT_Physics;
        else if (componentName == "Health") componentType = CT_Health;
        else if (componentName == "Tag") componentType = CT_Tag;
        else if (componentName == "Shape") componentType = CT_Shape;
        else if (componentName == "Input") componentType = CT_Input;
        else if (componentName == "Animator") componentType = CT_Animator;*/

        /*if (componentType != CT_None) {
            e.componentData[componentType] = it.value().dump();
        }*/
        e.componentData[componentName] = it.value().dump();
    }
}

/**
 * @brief Serializes SavedScene to JSON object.
 *
 * @param j Reference to the JSON object to populate with scene data.
 * @param s SavedScene to serialize.
 */
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

/**
 * @brief Deserializes SavedScene from JSON object.
 *
 * @param j The JSON object containing scene data.
 * @param s Reference to SavedScene to populate with deserialized data.
 */
void JsonLoad::from_json(const json& j, SavedScene& s) {
    s.sceneName = j["sceneName"];
    s.entities.clear();

    for (const auto& entityJson : j["entities"]) {
        SavedEntity entity;
        from_json(entityJson, entity);
        s.entities.push_back(entity);
    }
}

/**
 * @brief Serializes GameSettings to JSON object.
 *
 * @param j Reference to the JSON object to populate with settings data.
 * @param s GameSettings to serialize.
 */
void JsonLoad::to_json(json& j, const GameSettings& s) {
    j = json{
        {"volume", s.volume},
        {"resolutionWidth", s.resolutionWidth},
        {"resolutionHeight", s.resolutionHeight},
        {"fullscreen", s.fullscreen}
    };
}

/**
 * @brief Deserializes GameSettings from JSON object.
 *
 * @param j The JSON object containing settings data.
 * @param s Reference to GameSettings to populate with deserialized data.
 */
void JsonLoad::from_json(const json& j, GameSettings& s) {
    j.at("volume").get_to(s.volume);
    j.at("resolutionWidth").get_to(s.resolutionWidth);
    j.at("resolutionHeight").get_to(s.resolutionHeight);
    j.at("fullscreen").get_to(s.fullscreen);
}
#pragma endregion Serialization

#pragma region SavingandLoading

/**
 * @brief Saves current game scene to JSON file.
 *
 * @param filepath The file path where the scene should be saved.
 * @param sceneName The name to assign to this scene for identification.
 * @return bool True if the scene was successfully saved, false if an error
 *         occurred during serialization or file writing.
 */
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
                if (Registry::Get().Has_Component<TransformComponent>(e)) std::cout << "  - Has Transform" << std::endl;
                if (Registry::Get().Has_Component<Sprite::SpriteComponent>(e)) std::cout << "  - Has Sprite" << std::endl;
                if (Registry::Get().Has_Component<Physics::PhysicsComponent>(e)) std::cout << "  - Has Physics" << std::endl;
                if (Registry::Get().Has_Component<HealthComponent>(e)) std::cout << "  - Has Health" << std::endl;
                if (Registry::Get().Has_Component<TagComponent>(e)) std::cout << "  - Has Tag" << std::endl;
                if (Registry::Get().Has_Component<InputComponent>(e)) std::cout << "  - Has Input" << std::endl;

                SavedEntity saved = Registry::Get().Save_Entity(e);
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

/**
 * @brief Loads game scene from JSON file.
 *
 * @param filepath The file path to the scene file to load.
 * @return bool True if the scene was successfully loaded, false if the file
 *         doesn't exist, is empty, contains invalid JSON, or an error occurred
 *         during deserialization.
 */
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
            Entity e = Registry::Get().Load_Entity(savedEntity);
            //Entity e = SavedToEntity(savedEntity);
            // std::cout << "Created entity ID: " << e << " (was: " << savedEntity.id << ")" << std::endl;
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading scene: " << e.what() << std::endl;
        return false;
    }
}

bool JsonLoad::SaveSettings(const std::string& filepath, GameSettings& settings) {
    
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

/**
 * @brief Loads game settings from JSON file.
 *
 * @param filepath The file path to the settings file to load.
 * @return std::optional<GameSettings> Contains the loaded settings
 *         if successful, or std::nullopt if fails.
 */
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
/**
 * @brief Saves a single entity to a JSON file as a prefab.
 * @param filepath The destination file path to save to.
 * @param eID The entity ID to serialize.
 * @return bool True if save successful, false otherwise.
 */
bool JsonLoad::SaveEntity(const std::string& filepath, Entity eID) {
    try {
        // Get entity name from registry if possible, otherwise use default
        std::string entityName = "Entity_" + std::to_string(eID);

        // Save entity data
        SavedEntity saved = Registry::Get().Save_Entity(eID);

        // Wrap in a SavedScene structure so LoadEntityFromFile can read it
        SavedScene scene;
        scene.sceneName = saved.name;  // Use entity name as scene name
        scene.entities.push_back(saved);  // Add entity to entities array

        // Serialize the scene (not just the entity)
        json j;
        to_json(j, scene);

        // Write to file
        return FileIO::WriteFile(filepath, j.dump(4));
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving entity prefab: " << e.what() << std::endl;
        return false;
    }

}
/**
 * @brief Loads a single entity from a JSON prefab file.
 *
 * Deserializes entity data from a scene JSON file and overrides the
 * transform position with provided coordinates. Automatically updates
 * physics hitboxes to match the new position.
 *
 * @param filepath The prefab JSON file path.
 * @param x The x-coordinate to spawn the entity at.
 * @param y The y-coordinate to spawn the entity at.
 * @return Entity The created entity ID, or 0 if failed.
 */

Entity JsonLoad::LoadEntityFromFile(const std::string& filepath, float x, float y) {
    std::string content = FileIO::ReadFile(filepath);
    if (content.empty()) {
        std::cerr << "Failed to load entity from: " << filepath << std::endl;
        return 0;
    }

    try {
        json j = json::parse(content);
        SavedScene scene;
        from_json(j, scene);  // Can call private function from within the class

        if (scene.entities.empty()) {
            std::cerr << "No entities in file: " << filepath << std::endl;
            return 0;
        }

        Entity e = Registry::Get().Load_Entity(scene.entities[0]);
        //Entity e = SavedToEntity(scene.entities[0]);  // Can call private function

        // Override position
        if (Registry::Get().Has_Component<TransformComponent>(e)) {
            auto& transform = Registry::Get().Get_Component<TransformComponent>(e);
            transform.pos = { x, y };

            if (Registry::Get().Has_Component<Physics::PhysicsComponent>(e)) {
                auto& phys = Registry::Get().Get_Component<Physics::PhysicsComponent>(e);
                phys.UpdateHitboxes({ x, y });
            }
        }

        return e;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error loading entity: " << ex.what() << std::endl;
        return 0;
    }
}
#pragma endregion SavingandLoading

/**
 * @brief Clears all entities from the current scene.
 */
void JsonLoad::ClearAllEntities() {
    EntityManager& em = EntityManager::get();
    std::vector<Entity> toRemove;

    // get all alive entities
    for (Entity e = 0; e < em.maxEntities(); ++e) {
        if (em.isAlive(e)) {
            toRemove.push_back(e);
        }
    }

    // remove them one by one
    for (Entity e : toRemove) {
        Registry::Get().Remove_Entity(e);
    }

    std::cout << "Cleared " << toRemove.size() << " entities" << std::endl;
}