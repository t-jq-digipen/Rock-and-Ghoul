/* ========================================================
 * @file    AudioCache.cpp
 * @author  Tan Ying Clarissa | t.yingclarissa@digipen.edu
 * @date    15 OCT 2025
 *
 * @brief   Implementation file for the AudioCache asset management system
 *
 * This file contains the implementation of the AudioCache class, which
 * provides centralized management of audio assets through an audio manifest
 * JSON file. The JSON manifest file defines audio assets organized by category
 * (music, sfx, ui, ambience). The cache implements a bank system for batch
 * loading/unloading of related audio assets.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include "AudioCache.h"
#include "Audio.h"
#include "FileIO.h"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Static member initialization
std::unordered_map<std::string, AudioAsset> AudioCache::assetsByName;  // Look up assets by name
std::unordered_map<int, std::string> AudioCache::idToName;             // Convert IDs back to names
std::unordered_map<std::string, std::vector<std::string>> AudioCache::banks;  // Groups of related assets
int AudioCache::nextID = 0;  // Counter for assigning unique IDs

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
bool AudioCache::Initialize(const std::string& manifestPath) {
    // parse manifest file
    if (!ParseManifest(manifestPath)) {
        std::cerr << "[AudioCache] Failed to parse manifest!" << std::endl;
        return false;
    }

    return true;
}

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
bool AudioCache::ParseManifest(const std::string& manifestPath) {
    try {
        // Read the entire JSON file into string
        std::string content = FileIO::ReadFile(manifestPath);

        if (content.empty()) {
            std::cerr << "[AudioCache] Manifest file is empty or doesn't exist: " << manifestPath << std::endl;
            return false;
        }

        // Parse  JSON string into JSON object 
        json manifest = json::parse(content);

        // Start ID numbering from 0
        nextID = 0;

        // Helper lambda function to parse one category (music, sfx, etc.), avoid repeating the same code 4 times
        auto parseCategory = [&](const std::string& categoryName) {
            // Check if category exists in manifest
            if (!manifest.contains(categoryName)) return;

            // Get category section from JSON
            const json& category = manifest[categoryName];

            // Loop through each audio asset in category
            for (auto it = category.begin(); it != category.end(); ++it) {
                // Get asset name--key
                const std::string& assetName = it.key();
                // Get asset properties--value
                const json& assetData = it.value();

                // Create new AudioAsset
                AudioAsset asset;
                asset.name = assetName;
                asset.id = nextID++;  // Assign ID and increment for next asset
                asset.path = assetData["path"];
                asset.loop = assetData["loop"];
                asset.group = assetData["group"];

                // USE DEFAULTS IF NO SPATIAL AUDIO SPECIFIED
                asset.spatial = assetData.value("spatial", false);  // 3D sound or regular 2D?
                asset.minDistance = assetData.value("minDistance", 1.0f);  // Full volume distance
                asset.maxDistance = assetData.value("maxDistance", 100.0f);  // Silent distance

                // only load when need set null first
                asset.sound = nullptr;

                // store in map for easy lookup
                assetsByName[asset.name] = asset;
            }
            };

        // Parse all four categories of audio
        parseCategory("music");
        parseCategory("sfx");
        parseCategory("ui");
        parseCategory("ambience");

        // Parse banks
        if (manifest.contains("banks")) {
            const json& banksJson = manifest["banks"];

            // Loop thru each bank definition
            for (auto it = banksJson.begin(); it != banksJson.end(); ++it) {
                const std::string& bankName = it.key();  // Bank name like "level1"
                const json& assetList = it.value();       // List of asset names in this bank

                // Build a vector of asset names for this bank
                std::vector<std::string> assets;
                for (const auto& assetName : assetList) {
                    assets.push_back(assetName.get<std::string>());
                }

                // Store the bank in our banks map
                banks[bankName] = assets;
            }
        }

        return true;
    }
    catch (const std::exception& e) {
        // catch bad json or missing fields here
        std::cerr << "[AudioCache] Error parsing manifest: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief Shuts down the AudioCache and releases all loaded audio resources.
 */
void AudioCache::Shutdown() {
    // DON'T call release() bc FMOD system close handles cleanup
    for (auto& [name, asset] : assetsByName) {
        asset.sound = nullptr;  // clear ptr but don't release
    }

    assetsByName.clear();
    idToName.clear();
    banks.clear();
}

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
AudioAsset* AudioCache::GetAsset(const std::string& name) {
    // Try to find the asset in our map
    auto it = assetsByName.find(name);

    // If found, return a pointer to the asset
    if (it != assetsByName.end()) {
        return &it->second;  // it->second is the AudioAsset value in map
    }

    return nullptr;
}

std::string AudioCache::GetAssetName(int id) {
    auto it = idToName.find(id);
    if (it == idToName.end()) { return ""; }
    return it->second;
}

/**
 * @brief Retrieves a loaded FMOD::Sound pointer by asset name.
 * If the asset exists but hasn't been loaded yet, this function automatically
 * loads it. If the asset doesn't exist in the cache or fails, nullptr is returned.
 *
 * @param name The name identifier of the audio asset as defined in the manifest.
 * @return FMOD::Sound* Pointer to the loaded FMOD sound object, or nullptr
 *         if the asset doesn't exist or failed to load.
 */
FMOD::Sound* AudioCache::GetSound(const std::string& name) {
    // get the asset name
    AudioAsset* asset = GetAsset(name);

    // If asset exists AND sound is already loaded, return it
    if (asset && asset->sound) {
        return asset->sound;
    }

    // If asset exists but sound isn't loaded yet, load now
    if (asset && !asset->sound) {
        if (LoadAsset(name)) {
            return asset->sound;
        }
    }

    return nullptr; // loading failed
}

/**
 * @brief Checks if an audio asset has been loaded into memory.
 * @param name The name identifier of the audio asset.
 * @return bool True if the asset exists and its FMOD::Sound is loaded,
 *         false otherwise.
 */
bool AudioCache::IsLoaded(const std::string& name) {
    // Get the asset
    AudioAsset* asset = GetAsset(name);

    // Return true only if asset exists AND its sound pointer is not null
    return (asset && asset->sound != nullptr);
}

/**
 * @brief Explicitly loads an audio asset into memory.
 * @param name The name identifier of the audio asset to load.
 * @return bool True if the asset was loaded successfully or was already
 *         loaded, false if the asset doesn't exist or loading failed.
 */
bool AudioCache::LoadAsset(const std::string& name) {
    // Get the asset metadata
    AudioAsset* asset = GetAsset(name);

    // asset no exist
    if (!asset) {
        std::cerr << "[AudioCache] Asset not found: " << name << std::endl;
        return false;
    }

    // already loaded
    if (asset->sound) {
        return true;
    }

    // not loaded yet...
    return LoadAudioFile(*asset);
}

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
bool AudioCache::LoadAudioFile(AudioAsset& asset) {
    // Start with either 3D or 2D mode based on asset settings
    FMOD_MODE mode = asset.spatial ? FMOD_3D : FMOD_2D;

    // | means "add this flag too"
    if (asset.loop)
        mode |= FMOD_LOOP_NORMAL;  // Loop forever
    else
        mode |= FMOD_LOOP_OFF;     // Play once and stop

    // If spatial, add linear rolloff (sound gets quieter with distance)
    if (asset.spatial)
        mode |= FMOD_3D_LINEARROLLOFF; // fmod so cool

    // load the actual file
    asset.sound = AudioSystem::LoadSound(asset.path, mode);

    if (!asset.sound) {
        std::cerr << "[AudioCache] Failed to load: " << asset.path << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief Unloads an audio asset from memory.
 * @param name The name identifier of the audio asset to unload.
 */
void AudioCache::UnloadAsset(const std::string& name) {
    // Get the asset
    AudioAsset* asset = GetAsset(name);

    // nothing to unload
    if (!asset) return;

    // release it from FMOD
    if (asset->sound) {
        asset->sound->release();  // Free FMOD memory
        asset->sound = nullptr;   // Clear our pointer
    }
}

// ============================================================================
// BANK SYSTEM FUNCTIONS
// ============================================================================

/**
 * @brief Loads all audio assets in a named bank. For example, "init" bank
 * would contain main menu music and button click SFX.
 * @param bankName The name identifier of the bank as defined in the manifest.
 * @return bool True if all assets in the bank loaded successfully, false if
 *         the bank doesn't exist or any asset failed to load.
 */
bool AudioCache::LoadBank(const std::string& bankName) {
    // find the bank
    auto it = banks.find(bankName);

    if (it == banks.end()) { // reach end and no bank
        std::cerr << "[AudioCache] Bank not found: " << bankName << std::endl;
        return false;
    }

    // Retrieve list of asset names belonging to bank
    const std::vector<std::string>& assets = it->second;
    bool allLoaded = true; // assets all loadded successfully

    // Loop through each asset name in the bank
    for (const std::string& assetName : assets) {
        // Try to load an asset
        if (!LoadAsset(assetName)) {
            std::cerr << "[AudioCache] Failed to load asset in bank: " << assetName << std::endl;

            allLoaded = false;
        }
    }

    // Return true only if ALL assets loaded, otherwise false
    return allLoaded;
}

/**
 * @brief Unloads all audio assets in a named bank.
 * @param bankName The name identifier of the bank as defined in the manifest.
 */
void AudioCache::UnloadBank(const std::string& bankName) {
    auto it = banks.find(bankName);

    if (it == banks.end()) {
        std::cerr << "[AudioCache] Bank not found: " << bankName << std::endl;
        return;
    }

    // list of assets in bank
    const std::vector<std::string>& assets = it->second;

    // Unload one by one
    for (const std::string& assetName : assets) {
        UnloadAsset(assetName);
    }
}

/**
 * @brief Retrieves the list of asset names in a bank.
 * @param bankName The name identifier of the bank.
 * @return std::vector<std::string> Vector of asset names in the bank, or
 *         an empty vector if the bank doesn't exist.
 */
std::vector<std::string> AudioCache::GetBankAssets(const std::string& bankName) {
    auto it = banks.find(bankName);

    // return the list of asset names
    if (it != banks.end()) {
        return it->second;
    }

    // Bank doesn't exist, return empty vector
    return {};
}

/**
 * @brief Retrieves a reference to all registered audio assets. Useful for
 * editor tools, debugging, or populating UI elements with available audio
 * assets.
 * @return const std::unordered_map<std::string, AudioAsset>& Reference to
 *         the complete asset registry map.
 */
const std::unordered_map<std::string, AudioAsset>& AudioCache::GetAllAssets() {
    // return entire map for editor to show all available sounds
    return assetsByName;
}