/* ========================================================
 * @file    AudioCache.h
 * @author  Tan Ying Clarissa | t.yingclarissa@digipen.edu
 * @date    15 OCT 2025
 *
 * @brief   Header file for centralized audio asset management system
 *
 * This file defines the AudioCache class and AudioAsset structure,
 * which together provide a manifest-based audio resource management
 * system. It provides centralized management of audio assets through
 * an audio manifest JSON file. The JSON manifest file defines audio
 * assets organized by category (music, sfx, ui, ambience). The cache
 * implements a bank system for batch loading/unloading of related audio assets.
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "fmod.hpp"

/**
 * @struct AudioAsset
 * @brief Container for audio asset metadata and FMOD sound handle.
 *
 * This structure stores all information about a single audio asset including
 * its unique identifier, file path, playback settings (loop, spatial audio),
 * distance attenuation parameters for 3D audio, and the loaded FMOD::Sound
 * pointer. Assets are populated from the JSON manifest during initialization.
 */
struct AudioAsset {
    int id;
    std::string name;
    std::string path;
    std::string group;
    bool loop;
    bool spatial;
    float minDistance;
    float maxDistance;
    FMOD::Sound* sound;

    AudioAsset() : id(-1), loop(false), spatial(false), minDistance(1.0f), maxDistance(100.0f), sound(nullptr) {}
};

/**
 * @enum AudioGroup
 * @brief Categorization of audio assets by functional type.
 * This enumeration defines the main categories for organizing audio assets.
 */
enum class AudioGroup {
    MUSIC,
    SFX,
    UI,
    AMBIENCE
};

class AudioCache {
public:
    /**
     * @brief Retrieves a reference to all registered audio assets. Useful for
     * editor tools, debugging, or populating UI elements with available audio
     * assets.
     * @return const std::unordered_map<std::string, AudioAsset>& Reference to
     *         the complete asset registry map.
     */
    static const std::unordered_map<std::string, AudioAsset>& GetAllAssets(); // for editor

    /**
     * @brief Initializes the AudioCache system by parsing the audio manifest file.
     *
     * This function loads and parses a JSON manifest file. The manifest is parsed to
     * fill up the internal asset registry, but audio files are not loaded into memory
     * until requested.
     *
     * @param manifestPath File path to the JSON audio manifest file.
     * @return bool True if the manifest was successfully parsed and the cache
     *         initialized, false if parsing failed or the file was not found.
     */
    static bool Initialize(const std::string& manifestPath);
    /**
     * @brief Shuts down the AudioCache and releases all loaded audio resources.
     */
    static void Shutdown();

    /**
     * @brief Retrieves an AudioAsset pointer by name.
     *
     * This function looks up an audio asset in the cache by name string.
     * The returned AudioAsset contains data about the audio file including
     * its path, loop settings, spatial audio properties, and a pointer to the
     * loaded FMOD::Sound.
     *
     * @param name The name identifier of the audio asset as defined in the manifest.
     * @return AudioAsset* Pointer to the AudioAsset if found, or nullptr if
     *         no asset with the given name exists in the cache.
 */
    static AudioAsset* GetAsset(const std::string& name);

    /**
     * @brief Retrieves a loaded FMOD::Sound pointer by asset name.
     * If the asset exists but hasn't been loaded yet, this function automatically
     * loads it. If the asset doesn't exist in the cache or fails, nullptr is returned.
     *
     * @param name The name identifier of the audio asset as defined in the manifest.
     * @return FMOD::Sound* Pointer to the loaded FMOD sound object, or nullptr
     *         if the asset doesn't exist or failed to load.
     */
    static FMOD::Sound* GetSound(const std::string& name);

    /**
     * @brief Checks if an audio asset has been loaded into memory.
     * @param name The name identifier of the audio asset.
     * @return bool True if the asset exists and its FMOD::Sound is loaded,
     *         false otherwise.
     */
    static bool IsLoaded(const std::string& name);
    /**
     * @brief Explicitly loads an audio asset into memory.
     * @param name The name identifier of the audio asset to load.
     * @return bool True if the asset was loaded successfully or was already
     *         loaded, false if the asset doesn't exist or loading failed.
     */
    static bool LoadAsset(const std::string& name);
    /**
     * @brief Unloads an audio asset from memory.
     * @param name The name identifier of the audio asset to unload.
     */
    static void UnloadAsset(const std::string& name);

    /**
     * @brief Loads all audio assets in a named bank. For example, "init" bank
     * would contain main menu music and button click SFX.
     * @param bankName The name identifier of the bank as defined in the manifest.
     * @return bool True if all assets in the bank loaded successfully, false if
     *         the bank doesn't exist or any asset failed to load.
     */
    static bool LoadBank(const std::string& bankName);
    /**
     * @brief Unloads all audio assets in a named bank.
     * @param bankName The name identifier of the bank as defined in the manifest.
     */
    static void UnloadBank(const std::string& bankName);
    /**
     * @brief Retrieves the list of asset names in a bank.
     * @param bankName The name identifier of the bank.
     * @return std::vector<std::string> Vector of asset names in the bank, or
     *         an empty vector if the bank doesn't exist.
     */
    static std::vector<std::string> GetBankAssets(const std::string& bankName);

private:
    static int nextID; // to auto assign ids for audio

    static std::unordered_map<std::string, AudioAsset> assetsByName;
    static std::unordered_map<int, std::string> idToName;

    /**
     * @brief Parses the JSON audio manifest file and populates the asset registry.
     *
     * This function reads and parses a JSON manifest file containing audio asset
     * definitions organized by category (music, sfx, ui, ambience). For each
     * asset, it extracts data like name, file path, loop settings,
     * group classification, and spatial audio parameters (minDistance, maxDistance).
     *
     * @param manifestPath File path to the JSON audio manifest file.
     * @return bool True if parsing succeeded and all assets were registered,
     *         false if the file could not be read or JSON parsing failed.
     */
    static bool ParseManifest(const std::string& manifestPath);
    /**
     * @brief Loads an audio file into FMOD using asset data such as
     * looping behaviour. Loaded sound is stored in the asset's
     * sound pointer.
     *
     * @param asset Reference to the AudioAsset to load. The asset's sound
     *              pointer will be updated with the loaded FMOD::Sound.
     * @return bool True if the audio file was successfully loaded into FMOD,
     *         false if loading failed (e.g., file not found, unsupported format).
     */
    static bool LoadAudioFile(AudioAsset& asset);

    static std::unordered_map<std::string, std::vector<std::string>> banks;
};