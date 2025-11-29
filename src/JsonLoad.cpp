/* ========================================================
 * @file        JsonLoad.cpp
 * @author      Yasmin Tazi | yasmin.t@digipen.edu
 * @co-author   Tan Ying Clarissa | t.yingclarissa@digipen.edu
 * @date        04 NOV 2025
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
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include "JsonLoad.h"
#include "FileIO.h"
#include "Registry.h"
#include "Render.h"
#include "AudioComponent.h"
#include <iostream>
#include "RenderSystem.h"
#include "GuiSystem.h"
#include "CoreEngine.h"
#include "SceneManager.h"
#include "UndoSystem.h"
#include "Camera.h"
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
   //    ActiveFrames frames;
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
   //            ActiveFrames stateFrames;
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

        // Skip empty component data to avoid parse errors
        if (data.empty()) {
            continue;
        }

        // Store component data as string (no parsing needed during save)
        j["components"][componentName] = data;
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
        e.componentData[componentName] = it.value().get<std::string>();
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
        {"backgrounds", s.backgroundTextures},
        {"fontConfig", s.fontConfigPath},
        {"bgmAssetName", s.bgmAssetName}, // for audio to stop during edit mode

         {"gui", {
             {"buttons", json::array()},
             {"textboxes", json::array()},
             {"dialogues", json::array()}
         }},
        {"entities", json::array()}

    };

    // UI Buttons
    for (const auto& btn : s.guiButtons) {
        j["gui"]["buttons"].push_back({
            {"ID", btn.ID},
            {"label", btn.label},
            {"pos", {btn.pos.x, btn.pos.y}},
            {"size", {btn.size.x, btn.size.y}},
            //{"color", {btn.color.r, btn.color.g, btn.color.b}},
            {"renderData", btn.renderData},
            //{"shape", btn.shape},
            {"action", btn.action},
            {"actionName", btn.actionName},
            {"actionParams", btn.actionParams},
            {"soundType", btn.soundType }
            });
    }

    // UI Textboxes
    for (const auto& txt : s.guiTextboxes) {
        j["gui"]["textboxes"].push_back({
            {"ID", txt.ID},
            {"text", txt.text},
            {"pos", {txt.pos.x, txt.pos.y}},
            {"scale", txt.scale},
            {"color", {txt.color.r, txt.color.g, txt.color.b}},
            {"centered", txt.centered}
        });
    }

    // UI Dialogues
    for (const auto& dlg : s.guiDialogues) {
        const auto& tb = dlg.textBox;
        json jtb = {
            {"text", tb.text},
            {"pos", {tb.pos.x, tb.pos.y}},
            {"scale", tb.scale},
            {"color", {tb.color.r, tb.color.g, tb.color.b}},
            {"centered", tb.centered}
        };
        json jd = {
            {"ID", dlg.ID},
            {"textBox", std::move(jtb)},
            {"jsonPath", dlg.jsonPath},
            {"sceneID", dlg.sceneID.empty() ? "0" : dlg.sceneID},
            {"lines", dlg.lines},
            {"currentLineIndex", dlg.currentLineIndex}
        };
        j["gui"]["dialogues"].push_back(std::move(jd));
    }

    // UI Displays
    for (const auto& dp : s.guiDisplays) {
        j["gui"]["displays"].push_back({
            {"ID", dp.ID},
            {"pos", {dp.pos.x, dp.pos.y}},
            {"size", {dp.size.x, dp.size.y}},
            {"renderData", dp.renderData},
            {"centered", dp.centered}
        });
    }

    // GUI Groups
    for (const auto& group : GuiSystem::Get().GetGroups())
    {
        j["gui"]["groups"].push_back({
            {"ID", group.groupID},
            {"name", group.groupName },
            {"members", group.memberIDs}
        });
    }

    for (const auto& entity : s.entities) {
        json entityJson;
        to_json(entityJson, entity);
        j["entities"].push_back(entityJson);
    }


    j["cameraSpawn"] = { s.camSettings.pos.x, s.camSettings.pos.y };
    j["cameraZoom"] = s.camSettings.zoom;
    j["worldMin"] = { s.camSettings.worldMin.x, s.camSettings.worldMin.y };
    j["worldMax"] = { s.camSettings.worldMax.x,s.camSettings.worldMax.y };

    j["sceneType"] = static_cast<int>(s.sceneType);
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
    if (j.contains("bgmAssetName")) {
        s.bgmAssetName = j["bgmAssetName"];
    }
    else {
        s.bgmAssetName = "";  // NO BGM FOR THIS SCENE
    }
    if (j.contains("backgrounds")) {
        s.backgroundTextures = j["backgrounds"].get<std::vector<std::string>>();
    }
    if (j.contains("fontConfig"))
        s.fontConfigPath = j["fontConfig"];

    int fallbackID = 0; // running global order if missing

    // GUI stuff
    GuiSystem::Get().Clear();
    if (j.contains("gui"))
    {
        // BUTTONS
        if (j["gui"].contains("buttons")) {
            for (auto& btn : j["gui"]["buttons"]) {
                SavedScene::SavedButton b;
                b.ID = btn.value("ID", fallbackID++);
                b.label = btn["label"];
                b.pos = { btn["pos"][0], btn["pos"][1] };
                b.size = { btn["size"][0], btn["size"][1] };
                //b.color = { btn["color"][0], btn["color"][1], btn["color"][2] };
                if (btn.contains("renderData"))
                {
                    b.renderData = btn["renderData"];
                }
                if (btn.contains("actionName")) {
                    b.actionName = btn["actionName"];
                }
                else { b.actionName = "{}"; }
                if (btn.contains("actionParams")) {
                    b.actionParams = btn["actionParams"];
                }
                else { b.actionParams = "{}"; }
                //b.shape = btn["shape"];
                b.action = btn.value("action", "");
                b.soundType = btn.value("soundType", 0);
                s.guiButtons.push_back(b);
            }
        }
        // TEXT BOXES
        if (j["gui"].contains("textboxes")) {
            for (auto& txt : j["gui"]["textboxes"]) {
                SavedScene::SavedTextBox t;
                t.ID = txt.value("ID", fallbackID++);
                t.text = txt["text"];
                t.pos = { txt["pos"][0], txt["pos"][1] };
                t.scale = txt["scale"];
                t.color = { txt["color"][0], txt["color"][1], txt["color"][2] };
                t.centered = { txt["centered"] };
                s.guiTextboxes.push_back(t);
            }
        }
        // DIALOGUES
        if (j["gui"].contains("dialogues"))
        {
            for (auto& dlg : j["gui"]["dialogues"]) {
                SavedScene::SavedDialogue d;
                d.ID = dlg.value("ID", fallbackID++);

                // carry visual/textbox look
                d.textBox.text = dlg["textBox"]["text"];
                d.textBox.pos = { dlg["textBox"]["pos"][0], dlg["textBox"]["pos"][1] };
                d.textBox.scale = dlg["textBox"]["scale"];
                d.textBox.color = { dlg["textBox"]["color"][0], dlg["textBox"]["color"][1], dlg["textBox"]["color"][2] };
                d.textBox.centered = dlg["textBox"]["centered"];

                d.jsonPath = dlg["jsonPath"];
                d.sceneID = dlg.value("sceneID", std::string{ "0" });

                if (dlg.contains("lines") && dlg["lines"].is_array())
                {
                    d.lines.clear();
                    for (const auto& ln : dlg["lines"]) d.lines.push_back(ln.get<std::string>());
                }
                d.currentLineIndex = dlg.value("currentLineIndex", 0);

                s.guiDialogues.push_back(std::move(d));
            }
        }
        // DISPLAYS
        if (j["gui"].contains("displays"))
        {
            for (auto& dp : j["gui"]["displays"])
            {
                SavedScene::SavedDisplay d;
                d.ID = dp.value("ID", fallbackID++);
                d.pos = { dp["pos"][0], dp["pos"][1] };
                d.size = { dp["size"][0], dp["size"][1] };
                if (dp.contains("renderData"))
                {
                    d.renderData = dp["renderData"];
                }
                d.centered = { dp["centered"] };

                s.guiDisplays.push_back(std::move(d));
            }
        }
        // GROUPS
        if (j["gui"].contains("groups"))
        {
            for (auto& group : j["gui"]["groups"])
            {
                UIGroup temp;
                temp.groupID = group["ID"];
                temp.groupName = group["name"];
                for (int member : group["members"])
                {
                    temp.memberIDs.push_back(member);
                }
                GuiSystem::Get().AddGroup(temp);
            }
        }
    }
    

    for (const auto& entityJson : j["entities"]) {
        SavedEntity entity;
        from_json(entityJson, entity);
        s.entities.push_back(entity);
    }
    // ----------------------------------------------------
      // SAFE CAMERA LOADING (handles missing JSON fields)
      // ----------------------------------------------------
    Camera& cam = Camera::GetInstance();

    // 1. Load cameraSpawn (only override if JSON includes it)
    if (j.contains("cameraSpawn") && j["cameraSpawn"].is_array() && j["cameraSpawn"].size() == 2) {
        s.camSettings.pos.x = j["cameraSpawn"][0];
        s.camSettings.pos.y = j["cameraSpawn"][1];
        cam.SetPosition(s.camSettings.pos);   // APPLY only if JSON had it
    }
    else {
        // Keep whatever camera was already set (do NOT override)
        s.camSettings.pos = cam.GetPosition();
    }

    // 2. Load cameraZoom (only override if JSON includes it)
    if (j.contains("cameraZoom")) {
        s.camSettings.zoom = j["cameraZoom"];
        cam.SetZoom(s.camSettings.zoom);      // APPLY only if JSON had it
    }
    else {
        s.camSettings.zoom = cam.GetZoom();
    }

    // 3. Load worldMin
    if (j.contains("worldMin") && j["worldMin"].is_array() && j["worldMin"].size() == 2) {
        s.camSettings.worldMin.x = j["worldMin"][0];
        s.camSettings.worldMin.y = j["worldMin"][1];
    }
    else {
        // your world bounds fallback is fine
        s.camSettings.worldMin = { 0.f, 0.f };
    }

    // 4. Load worldMax
    if (j.contains("worldMax") && j["worldMax"].is_array() && j["worldMax"].size() == 2) {
        s.camSettings.worldMax.x = j["worldMax"][0];
        s.camSettings.worldMax.y = j["worldMax"][1];
    }
    else {
        s.camSettings.worldMax = { 1920.f, 1080.f };
    }

    // Only apply bounds if JSON had both
    if (j.contains("worldMin") && j.contains("worldMax")) {
        cam.SetBounds(s.camSettings.worldMin, s.camSettings.worldMax);
    }

    if (j.contains("sceneType")) {
        s.sceneType = static_cast<SceneType>(j["sceneType"]);
    }
    else {
        s.sceneType = ST_MENU;
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
bool JsonLoad::SaveCurrentScene(const std::string& filepath, const std::string& sceneName, const SavedScene::SavedCamera& camSettings) {
    try {
        SavedScene scene;
        scene.sceneName = sceneName;
        scene.backgroundTextures = RenderSystem::GetBackgroundTexturePaths();
        scene.fontConfigPath = "../Assets/fontAssets.json";

        // PRESERVE BGM PLEASE
        scene.bgmAssetName = SceneManager::Get().GetCurrentSceneBGM();
        std::cout << "[SaveCurrentScene] Saving BGM: '" << scene.bgmAssetName << "'\n";

        // GUI Buttons
        auto& gui = GuiSystem::Get();
        for (auto& b : gui.GetButtons()) {
            SavedScene::SavedButton sb;
            sb.ID = b.ID;
            sb.label = b.label;
            sb.pos = b.basePos;
            sb.size = b.baseSize;
            //sb.color = b.color;
            sb.renderData = b.m_renderData.Serialize();
            //sb.shape = ButtonShapeToString(b.shape);
            sb.action = ButtonActionToString(b.action); 
            sb.actionName = b.actionName;
            sb.actionParams = b.actionParams;
            sb.soundType = static_cast<int>(b.soundType);
            scene.guiButtons.push_back(sb);
        }
        // GUI TextBoxes
        for (auto& t : gui.GetTextBoxes())
        {
            SavedScene::SavedTextBox st;
            st.ID = t.ID;
            st.text = t.text;
            st.pos = t.basePos;
            st.scale = t.baseScale;
            st.color = t.color;
            st.centered = t.centered;
            scene.guiTextboxes.push_back(st);
        }
        // GUI Dialogues
        for (auto& d : gui.GetDialogues())
        {
            SavedScene::SavedDialogue sd;
            sd.ID = d.ID;
            sd.textBox.text = d.textBox.text;
            sd.textBox.pos = d.textBox.basePos;
            sd.textBox.scale = d.textBox.baseScale;
            sd.textBox.color = d.textBox.color;
            sd.textBox.centered = d.textBox.centered;
            sd.jsonPath = d.jsonPath;
            sd.sceneID = d.sceneID.empty() ? "0" : d.sceneID;

            const TextSystem::TextComponent* comp = d.textComp ? d.textComp : d.owned.get();
            if (comp)
            {
                sd.lines = comp->GetLines();
                sd.currentLineIndex = comp->GetCurrentLineIndex();
            }

            scene.guiDialogues.push_back(std::move(sd));
        }
        // GUI Displays
        for (auto& dp : gui.GetDisplays())
        {
            SavedScene::SavedDisplay sd;
            sd.ID = dp.ID;
            sd.pos = dp.basePos;
            sd.size = dp.baseSize;
            sd.renderData = dp.m_renderData.Serialize();
            sd.centered = dp.centered;
            scene.guiDisplays.push_back(std::move(sd));
        }
        // Get all alive entities
        EntityManager& em = EntityManager::get();
        int entityCount = 0;
        for (Entity e = 0; e < em.maxEntities(); ++e) {
            if (em.isAlive(e)) {
                SavedEntity saved = Registry::Get().Save_Entity(e);
                if (!saved.componentData.empty()) {
                    scene.entities.push_back(saved);
                    entityCount++;
                }
            }
        }

        std::cout << "Total entities saved: " << entityCount << std::endl;

        scene.camSettings = camSettings;
        scene.sceneType = SceneManager::Get().GetSceneType();

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

        ClearAllEntities();

        json j = json::parse(content);
        SavedScene scene;
        from_json(j, scene);

        std::cout << "Loading scene: " << scene.sceneName << std::endl;

        SceneManager::Get().SetSceneType(scene.sceneType);

        // Load entities
        for (const auto& savedEntity : scene.entities) {
            Entity e = Registry::Get().Load_Entity(savedEntity);
        }

        // Load backgrounds
        RenderSystem::ClearBackgroundTextures();
        for (auto& path : scene.backgroundTextures) {
            GLuint tex = Render::GetInstance().LoadTexture(path);
            if (tex) {
                RenderSystem::AddBackgroundTexture(tex, path);
            }
        }

        // Load fonts
        Render::GetInstance().LoadFontAssetsFromJSON("../Assets/fontAssets.json");

        // Restore GUI buttons
        for (auto& b : scene.guiButtons) {
            ButtonConfig cfg;
            cfg.ID = b.ID;
            cfg.label = b.label;
            cfg.pos = b.pos;
            cfg.basePos = b.pos;
            cfg.size = b.size;
            cfg.baseSize = b.size;
            //cfg.color = b.color;
            if (!b.renderData.empty())
            {
                cfg.m_renderData = UI_RenderData::DeSerialize(b.renderData);
            }
            //cfg.shape = StringToButtonShape(b.shape);
            cfg.action = StringToButtonAction(b.action);
            cfg.actionName = b.actionName;
            cfg.actionParams = b.actionParams;
            cfg.soundType = static_cast<AudioSystem::ButtonType>(b.soundType);

            if (b.action == "LoadLevel1") {
                cfg.onClick = []() { SceneManager::Get().QueueLoadScene("../Assets/saves/level1.json"); };
            }
            else if (b.action == "Quit") {
                cfg.onClick = []() {
                    // let quit sound play
                    double startTime = glfwGetTime();
                    while (glfwGetTime() - startTime < 0.5) {
                        AudioSystem::Update();
                        glfwPollEvents();
                    }

                    // stop all audio BEFORE shutdown
                    AudioSystem::StopAll();
                    // give FMOD time to cleanup by updating several times lol
                    for (int i = 0; i < 10; ++i) {
                        AudioSystem::Update();
                        glfwPollEvents();
                    }

                    // QUIT NOW!!!
                    CoreEngine::GetInstance().Quit();
                    };
            }
            else if (b.action == "LoadMainMenu") {
                cfg.onClick = []() {
                    SceneManager::Get().QueueLoadScene("../Assets/saves/MainMenu.json");
                    };
            }
            else if (b.action == "OpenStateWindow") {
                cfg.onClick = []() {
                    GuiSystem::Get().DrawStateWindow(Render::GetInstance());
                    };
            }
            else {
                cfg.onClick = nullptr;
            }

            GuiSystem::Get().AddButton(cfg);
        }

        // Restore GUI TextBoxes
        for (auto& t : scene.guiTextboxes)
        {
            TextBoxConfig cfg;
            cfg.ID = t.ID;
            cfg.text = t.text;
            cfg.pos = t.pos;
            cfg.basePos = t.pos;
            cfg.scale = t.scale;
            cfg.baseScale = t.scale;
            cfg.color = t.color;
            cfg.centered = t.centered;
            GuiSystem::Get().AddTextBox(cfg);
        }

        // Restore GUI Dialogues
        for (auto& d : scene.guiDialogues)
        {
            DialogueConfig cfg{};
            cfg.ID = d.ID;
            cfg.textBox.text = d.textBox.text;
            cfg.textBox.pos = d.textBox.pos;
            cfg.textBox.basePos = d.textBox.pos;
            cfg.textBox.scale = d.textBox.scale;
            cfg.textBox.baseScale = d.textBox.scale;
            cfg.textBox.color = Colour3 { d.textBox.color.r, d.textBox.color.g, d.textBox.color.b };
            cfg.textBox.fontKey = "Marker";
            cfg.textBox.centered = d.textBox.centered;
            cfg.textBox.isVisible = true;

            cfg.jsonPath = d.jsonPath;
            cfg.sceneID = d.sceneID.empty() ? "0" : d.sceneID;

            // Always create a runtime TextComponent we can hydrate
            cfg.owned = std::make_shared<TextSystem::TextComponent>();
            auto* comp = cfg.owned.get();

            // Helper: load a scene's script from Dialogue.json by sceneID (used as fallback)
            auto loadLinesFromDialogueJson = [&](const std::string& sceneID,
                std::vector<std::string>& outLines,
                float& textSpeed, std::string& fontPath,
                float& fontSize, Vec3& color)
                {
                    outLines.clear();
                    textSpeed = 30.0f;
                    fontPath = "../Assets/fonts/Marker.ttf";
                    fontSize = 1.0f;
                    color = { 1.f, 1.f, 1.f };

                    try {
                        std::ifstream ifs(cfg.jsonPath);
                        if (!ifs) return;
                        json doc; ifs >> doc;
                        if (!doc.contains("scenes") || !doc["scenes"].is_array()) return;

                        // find scene by sceneID string
                        const auto& arr = doc["scenes"];
                        int found = -1;
                        for (int i = 0; i < (int)arr.size(); ++i)
                        {
                            const auto& sc = arr[i];
                            if (sc.contains("sceneID") && sc["sceneID"].is_string()
                                && sc["sceneID"].get<std::string>() == sceneID) {
                                found = i; break;
                            }
                        }
                        if (found < 0) return; // no match

                        const auto& sc = arr[found];

                        if (sc.contains("script") && sc["script"].is_array())
                        {
                            for (const auto& node : sc["script"])
                            {
                                if (node.contains("line") && node["line"].is_string())
                                    outLines.push_back(node["line"].get<std::string>());
                            }
                        }
                        if (outLines.empty()) outLines.push_back("");

                        textSpeed = sc.value("textSpeed", 30.0f);
                        fontPath = sc.value("fontPath", std::string("../Assets/fonts/Marker.ttf"));
                        fontSize = sc.value("fontSize", 1.0f);

                        if (sc.contains("color") && sc["color"].is_array() && sc["color"].size() == 3)
                        {
                            color = {
                                sc["color"][0].get<float>(),
                                sc["color"][1].get<float>(),
                                sc["color"][2].get<float>()
                            };
                        }
                    }
                    catch (...) {
                        // leave defaults
                    }
                };

            // Prefer saved snapshot; otherwise fallback to JSON by sceneID
            if (!d.lines.empty())
            {
                comp->SetLines(d.lines);
                int clamped = std::clamp(d.currentLineIndex, 0, (int)d.lines.size() - 1);
                comp->SetCurrentLineIndex(clamped);
                comp->SetIsTyping(false);        // editor idle by default
                // You may call comp->StopTyping() if you want the full line visible immediately.
            }
            else
            {
                std::vector<std::string> lines;
                float textSpeed = 30.f; std::string fontPath; float fontSize = 1.f; Vec3 color{};
                loadLinesFromDialogueJson(cfg.sceneID, lines, textSpeed, fontPath, fontSize, color);

                comp->SetLines(lines);
                comp->SetCurrentLineIndex(0);
                comp->SetIsTyping(false);

                comp->SetTextSpeed(textSpeed);
                comp->SetFontPath(fontPath);
                comp->SetFontSize(fontSize);
                comp->SetColor(color);

                // keep text color in sync with scene default
                cfg.textBox.color = { color.x, color.y, color.z };
            }

            GuiSystem::Get().AddDialogue(cfg);
        }

        // Restore GUI Displays
        for (auto& dp : scene.guiDisplays)
        {
            DisplayConfig cfg;
            cfg.ID = dp.ID;
            cfg.pos = dp.pos;
            cfg.basePos = dp.pos;
            cfg.size = dp.size;
            cfg.baseSize = dp.size;
            cfg.centered = dp.centered;
            if (!dp.renderData.empty())
            {
                cfg.m_renderData = UI_RenderData::DeSerialize(dp.renderData);
            }
            GuiSystem::Get().AddDisplay(cfg);
        }

        SavedScene::SavedCamera& camSet = scene.camSettings;

        // Camera settings
        Camera& cam = Camera::GetInstance();
        cam.SetPosition(scene.camSettings.pos);
        cam.SetZoom(scene.camSettings.zoom);
        cam.SetBounds(scene.camSettings.worldMin, scene.camSettings.worldMax);

        // set bgm before entering...
        SceneManager::Get().SetCurrentSceneBGM(scene.bgmAssetName);
        std::cout << "[LoadScene] Scene BGM set to: '" << scene.bgmAssetName << "'\n";

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

        //// Wrap in a SavedScene structure so LoadEntityFromFile can read it
        //SavedScene scene;
        //scene.sceneName = saved.name;  // Use entity name as scene name
        //scene.entities.push_back(saved);  // Add entity to entities array

        //// Serialize the scene (not just the entity)
        //json j;
        //to_json(j, scene);
        json j;
        to_json(j, saved);

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

        SavedEntity enty;
        from_json(j, enty);

        //Entity e = Registry::Get().Load_Entity(scene.entities[0]);
        Entity e = Registry::Get().Load_Entity(enty);
        //Entity e = SavedToEntity(scene.entities[0]);  // Can call private function

        // Override position
        if (Registry::Get().Has_Component<TransformComponent>(e)) {
            auto& transform = Registry::Get().Get_Component<TransformComponent>(e);
            transform.pos = { x, y };

            if (Registry::Get().Has_Component<ColliderComponent>(e)) {
                auto& collider = Registry::Get().Get_Component<ColliderComponent>(e);
                collider.UpdateHitboxes(transform.pos, transform.scale);
            }
        }

        return e;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error loading entity from file: " << filepath << std::endl;
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
    UndoSystem::Get().Clear();
    std::cout << "Cleared " << toRemove.size() << " entities" << std::endl;
}

/**
 * @brief Loads current game scene from JSON file.
 *
 * @param filepath The file path to the scene file to load.
 * @param sceneName The name to assign to this scene for identification.
 * @return bool True if the scene was successfully saved, false if an error
 *         occurred during serialization or file writing.
 */

/*bool JsonLoad::LoadCurrentScene(const std::string& filepath, const std::string& sceneName)
{
    try 
    {
        std::string content = FileIO::ReadFile(filepath);
        if (content.empty())
        {
            std::cerr << "File is empty or doesn't exist: " << filepath << std::endl;
            return false;
        }

        json j = json::parse(content);

        auto itScenes = j.find("scenes");
        if (itScenes == j.end() || !itScenes->is_array())
        {
            std::cerr << "Scene missing in: " << filepath << std::endl;
            return false;
        }

        const json* isMatched = nullptr;
        for (const auto& s : *itScenes)
        {
            if (!s.is_object()) continue;
            auto itSceneName = s.find("sceneName");
            if (itSceneName != s.end() && itSceneName->is_string() && itSceneName->get_ref<const std::string&>() == sceneName)
            {
                isMatched = &s;
                break;
            }
        }

        if (!isMatched)
        {
            std::cerr << "Scene '" << sceneName << "' not found in " << filepath << std::endl;
            return false;
        }

        SavedScene scene;
        scene.sceneName = isMatched->value("sceneName", std::string{});

        auto itEntities = isMatched->find("entities");
        if (itEntities != isMatched->end() && itEntities->is_array()) {
            for (const auto& ent : *itEntities) {
                if (!ent.is_object()) continue;

                SavedEntity save;
                save.name = ent.value("name", std::string{});

                auto itComps = ent.find("components");
                if (itComps != ent.end() && itComps->is_object()) {
                    for (auto it = itComps->begin(); it != itComps->end(); ++it) {
                        if (!it.value().is_string()) continue;
                        save.componentData[it.key()] = it.value().get<std::string>();
                    }
                }

                if (!save.componentData.empty()) {
                    scene.entities.push_back(std::move(save));
                }
            }
        }

        std::cout << "Loading scene: " << scene.sceneName << std::endl;
        std::cout << "Number of entities: " << scene.entities.size() << std::endl;

        // Clear existing entities before loading scene
        ClearAllEntities();

        // Create all entities
        for (const auto& savedEntity : scene.entities) {
            Registry::Get().Load_Entity(savedEntity);
        }

        return true;
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Error loading scene '" << sceneName << "' from '" << filepath << "': " << e.what() << std::endl;
        return false;
    }
}*/

/**
 * @brief Saves temporary scene to JSON file based on sceneName.
 *
 * @param filepath The file path to the scene file to load.
 * @param sceneName The name to assign to this scene for identification.
 * @return bool True if the temp scene was successfully saved, false if an error
 *         occurred during serialization or file writing.
 */

/*bool JsonLoad::SaveCurrentTempScene(const std::string& filepath, const std::string& sceneName)
{
    try {
        // 1) Snapshot current world into a SavedScene
        SavedScene scene;
        scene.sceneName = sceneName;

        EntityManager& em = EntityManager::get();
        for (Entity e = 0; e < em.maxEntities(); ++e) 
        {
            if (!em.isAlive(e)) continue;
            SavedEntity saved = Registry::Get().Save_Entity(e);
            if (!saved.componentData.empty()) 
            {
                scene.entities.push_back(std::move(saved));
            }
        }

        json jScene;
        to_json(jScene, scene); // -> {"sceneName": "...", "entities": [...]}

        // 2) Load existing file (or create a fresh root)
        json root = json::object();
        if (std::string content = FileIO::ReadFile(filepath); !content.empty()) 
        {
            try 
            {
                root = json::parse(content);
            }
            catch (const std::exception& e) 
            {
                // If file is corrupt, start fresh but keep going
                std::cerr << "Error saving temp scene: " << e.what() << '\n';
                root = json::object();
            }
        }
        if (!root.is_object()) root = json::object();

        // 3) Ensure scenes array exists
        if (!root.contains("scenes") || !root["scenes"].is_array())
            root["scenes"] = json::array();

        json& scenesIn = root["scenes"];
        json scenesOut = json::array();
        bool replaced = false;

        // 4) Rebuild scenes array; replace only the matching sceneName
        for (const auto& s : scenesIn) 
        {
            if (!s.is_object()) { scenesOut.push_back(s); continue; }

            // Safe lookup: do NOT use operator[] on const json
            auto it = s.find("sceneName");
            if (it != s.end() && it->is_string() && it->get_ref<const std::string&>() == sceneName) 
            {
                scenesOut.push_back(jScene);   // replace
                replaced = true;
            }
            else 
            {
                scenesOut.push_back(s);        // keep as-is
            }
        }
        if (!replaced) 
        {
            scenesOut.push_back(jScene);        // append new scene
        }

        // 5) Write back (preserves other scenes)
        root["scenes"] = std::move(scenesOut);
        return FileIO::WriteFile(filepath, root.dump(4));
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error in SaveCurrentTempScene('" << sceneName << "'): " << e.what() << '\n';
        return false;
    }
}*/
