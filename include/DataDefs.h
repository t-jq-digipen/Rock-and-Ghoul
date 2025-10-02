/* ========================================================
 * @file        DataDefs.h
 * @author      Yasmin Tazi         | yasmin.t@digipen.edu
 * @co-author   Tan Ying Clarissa   | t.yingclarissa@digipen.edu
 * @date        26 SEP 2025
 *
 * @brief   Data structures for game serialization
 *
 * This header file defines data structures for saving and
 * loading game data through the JSON serialization system.
 * These structures bridge the runtime ECS and persistent
 * storage formats.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Component.h"

 /**
  * @struct SavedEntity
  * @brief Serializable entity structure with component data.
  */
struct SavedEntity {
    int id;                                                      
    std::string name;                                           
    std::unordered_map<ComponentTypeID, std::string> componentData; 

    /**
     * @brief Default constructor with zero ID and empty name.
     */
    SavedEntity() : id(0), name("") {}
};

/**
 * @struct SavedPhysics
 * @brief Legacy physics serialization structure.
 */
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

/**
 * @struct SavedScene
 * @brief Serializable scene structure with all entities.
 */
struct SavedScene {
    std::string sceneName;  
    std::vector<SavedEntity> entities; 

    /**
     * @brief Default constructor with "Level1" name.
     */
    SavedScene() : sceneName("Level1") {}
};

/**
 * @struct GameSettings
 * @brief Persistent game configuration settings.
 */
struct GameSettings {
    float volume;      
    int resolutionWidth;     // Display width in pixels
    int resolutionHeight;    // Display height in pixels
    bool fullscreen;       

    /**
     * @brief Default constructor with 1920x1080 fullscreen off.
     */
    GameSettings() : volume(1.0f), resolutionWidth(1920), resolutionHeight(1080), fullscreen(false) {}
};