/* ========================================================
 * @file        JsonLoad.h
 * @author      Yasmin Tazi | yasmin.t@digipen.edu
 * @co-author   Tan Ying Clarissa | t.yingclarissa@digipen.edu
 * @date        26 SEP 2025
 *
 * @brief   Header file declaring the JSON serialization system
 *
 * This header file contains the declaration of the JsonLoad class,
 * which provides a comprehensive JSON-based serialization and
 * deserialization system for the game's Entity Component System (ECS).
 * It uses the nlohmann JSON library for parsing and serialization.
 *
 * Copyright � 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include "DataDefs.h"
#include <nlohmann/json.hpp>
#include <optional>
#include "Entity.h"

using json = nlohmann::json;

/**
 * @class JsonLoad
 * @brief Static utility class for JSON serialization of game data.
 *
 * The JsonLoad class converts game objects to JSON format for storage
 * and reconstructs them from saved files. The class handles all component
 * types in the ECS and ensures proper loading order.
 */
class JsonLoad {
public:
    // ===== MAIN FUNCTIONS =====

/**
 * @brief Saves the current game scene to a JSON file.
 *
 * Serializes all alive entities and their components into a JSON format
 * and writes to the specified filepath.
 *
 * @param filepath The destination file path.
 * @param sceneName The name identifier for the scene.
 * @return bool True if save successful, false otherwise.
 */
    static bool SaveCurrentScene(const std::string& filepath, const std::string& sceneName);
    
 /**
 * @brief Saves a single entity to a JSON file.
 *
 * Serializes the specified entity and all its components to JSON format.
 *
 * @param filepath The destination file path to save to.
 * @param eID The entity ID to serialize.
 * @return bool True if save successful, false otherwise.
 */
    static bool SaveEntity(const std::string& filepath, Entity eID);
 
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
    static Entity LoadEntityFromFile(const std::string& filepath, float x, float y);
    
    // Clear all entities before loading (no cloning)
    static void ClearAllEntities();

    /**
     * @brief Loads game scene from JSON file.
     *
     * @param filepath The file path to the scene file to load.
     * @return bool True if the scene was successfully loaded, false if the file
     *         doesn't exist, is empty, contains invalid JSON, or an error occurred
     *         during deserialization.
     */
    static bool LoadScene(const std::string& filepath);

    // Save game settings
    static bool SaveSettings(const std::string& filepath, GameSettings& settings);
    static std::optional<GameSettings> LoadSettings(const std::string& filepath);

private:
    /**
     * @brief Converts live entity to SavedEntity structure.
     *
     * @param eID The entity ID to convert to a SavedEntity.
     * @return SavedEntity Structure containing the entity's ID, name, and all
     *         component data as JSON strings indexed by component type.
     */
    //static SavedEntity EntityToSaved(Entity eID);

    /**
     * @brief Creates live entity from SavedEntity structure.
     *
     * @param saved The SavedEntity structure containing serialized component data.
     * @return Entity The ID of the newly created entity with all components attached.
     */
    // static Entity SavedToEntity(const SavedEntity& saved);

    // Component loaders

    /**
     * @brief Deserializes and loads a Transform component onto an entity.
     *
     * @param eID The entity ID to which the component should be added.
     * @param data JSON string containing transform data.
     */
    // static void Load_Transform(Entity eID, const std::string& data);

    /**
     * @brief Deserializes and loads Sprite component onto entity.
     *
     * @param eID The entity ID to which the Sprite component should be added.
     * @param data JSON string containing sprite data (texturePath, scale, animated).
     */
    // static void Load_Sprite(Entity eID, const std::string& data);

    /**
     * @brief Deserializes and loads a Physics component onto an entity.
     *
     * This function reconstructs the physics component including velocity,
     * collision settings, AABB and circle collider.
     *
     * @param eID The entity ID to which the component should be added.
     * @param data JSON string containing physics data.
     */
    // static void Load_Physics(Entity eID, const std::string& data);

    /**
     * @brief Deserializes and loads a Shape component onto an entity.
     *
     * @param eID The entity ID to which the component should be added.
     * @param data JSON string containing shape data.
     */
    //static void Load_Shape(Entity eID, const std::string& data);

    /**
     * @brief Deserializes and loads a Health component onto an entity.
     *
     * @param eID The entity ID to which the component should be added.
     * @param data JSON string containing health data.
     */
    //static void Load_Health(Entity eID, const std::string& data);

    /**
     * @brief Deserializes and loads a Tag component onto an entity.
     *
     * @param eID The entity ID to which the component should be added.
     * @param data JSON string containing tag data.
     */
    //static void Load_Tag(Entity eID, const std::string& data);

    /**
     * @brief Deserializes and loads an Input component onto an entity.
     *
     * @param eID The entity ID to which the component should be added.
     * @param data JSON string containing input data.
     */
    //static void Load_Input(Entity eID, const std::string& data);

    /**
     * @brief Deserializes and loads Animator component onto entity.
     *
     * @param eID The entity ID to which the Animator component should be added.
     * @param data JSON string containing animator data.
     * @throws std::runtime_error If the entity doesn't have a Sprite component.
     */
    // static void Load_Animator(Entity eID, const std::string& data);

    // Component savers

    /**
     * @brief Serializes a Transform component to JSON string.
     *
     * @param eID The entity ID whose Transform component should be serialized.
     * @return std::string JSON string containing transform data.
     */
    // static std::string Save_Transform(Entity eID);

    /**
     * @brief Serializes a Sprite component to JSON string.
     *
     * Extracts the texture path from the sprite's image data
     * with scale and animation settings.
     *
     * @param eID The entity ID whose Sprite component should be serialized.
     * @return std::string JSON string containing sprite data.
     */
    // static std::string Save_Sprite(Entity eID);

    /**
     * @brief Serializes Physics component to JSON string.
     *
     * @param eID The entity ID whose Physics component should be serialized.
     * @return std::string JSON string containing the physics data.
     */
    // static std::string Save_Physics(Entity eID);

    /**
     * @brief Serializes Shape component to JSON string.
     *
     * @param eID Entity ID Shape component should be serialized.
     * @return std::string JSON string containing the shape data
     */
    // static std::string Save_Shape(Entity eID);

    /**
     * @brief Serializes Health component to JSON string.
     *
     * @param eID The entity ID where Health component should be serialized.
     * @return std::string JSON string containing the health data.
     */
    // static std::string Save_Health(Entity eID);

    /**
     * @brief Serializes Tag component to JSON string.
     *
     * @param eID The entity ID whose Tag component should be serialized.
     * @return std::string JSON string containing the tag data.
     */
    //static std::string Save_Tag(Entity eID);

    /**
     * @brief Serializes Input component to JSON string.
     *
     * @param eID The entity ID where Input component should be serialized.
     * @return std::string JSON string containing the input data.
     */
    //static std::string Save_Input(Entity eID);
    
    /**
     * @brief Serializes Animator component to JSON string.
     *
     * @param eID The entity ID whose Animator component should be serialized.
     * @return std::string JSON string containing the animator data including:
     *         totalRows, totalCols, animDuration, frame configuration,
     *         currentState, all animation states, and UV coordinates (u0, v0, u1, v1).
     */
    // static std::string Save_Animator(Entity eID);

    // JSON serialization

    /**
     * @brief Serializes a SavedEntity to a JSON object.
     *
     * Converts a SavedEntity into a structured JSON object with
     * human-readable component names instead of component type IDs.
     *
     * @param j Reference to the JSON object to populate with entity data.
     * @param e SavedEntity to serialize.
     */
    static void to_json(json& j, const SavedEntity& e);

    /**
     * @brief Deserializes a SavedEntity from a JSON object.
     *
     * Parses a JSON object containing entity data and reconstructs
     * a SavedEntity structure, converting component names back to type IDs.
     *
     * @param j The JSON object containing entity data.
     * @param e Reference to SavedEntity to populate with deserialized data.
     */
    static void from_json(const json& j, SavedEntity& e);

    /**
     * @brief Serializes a SavedScene to a JSON object.
     *
     * Converts a SavedScene containing multiple entities into
     * a JSON object with all entities in an array.
     *
     * @param j Reference to the JSON object to populate with scene data.
     * @param s SavedScene to serialize.
     */
    static void to_json(json& j, const SavedScene& s);

    /**
     * @brief Deserializes a SavedScene from a JSON object.
     *
     * Parses a JSON object containing scene data and reconstructs
     * a SavedScene structure with all entities.
     *
     * @param j The JSON object containing scene data.
     * @param s Reference to SavedScene to populate with deserialized data.
     */
    static void from_json(const json& j, SavedScene& s);

    /**
     * @brief Serializes GameSettings to a JSON object.
     *
     * Function converts game settings (volume, resolution, fullscreen)
     * into a JSON object for storage.
     *
     * @param j Reference to the JSON object to populate with settings data.
     * @param s The GameSettings to serialize.
     */
    static void to_json(json& j, const GameSettings& s);

    /**
     * @brief Deserializes GameSettings from a JSON object.
     *
     * Parses a JSON object containing game settings and populates
     * a GameSettings structure with the values.
     *
     * @param j The JSON object containing settings data.
     * @param s Reference to the GameSettings to populate with deserialized data.
     */
    static void from_json(const json& j, GameSettings& s);
};