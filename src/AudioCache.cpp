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
std::unordered_map<std::string, AudioAsset> AudioCache::assetsByName;
std::unordered_map<int, std::string> AudioCache::idToName;
std::unordered_map<std::string, std::vector<std::string>> AudioCache::banks;
int AudioCache::nextID = 0;

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
        std::string content = FileIO::ReadFile(manifestPath);
        if (content.empty()) {
            std::cerr << "[AudioCache] Manifest file is empty or doesn't exist: " << manifestPath << std::endl;
            return false;
        }

        json manifest = json::parse(content);

        nextID = 0;
        
        // Helper to parse a category
        auto parseCategory = [&](const std::string& categoryName) {
            if (!manifest.contains(categoryName)) return;

            const json& category = manifest[categoryName];
            for (auto it = category.begin(); it != category.end(); ++it) {
                const std::string& assetName = it.key();
                const json& assetData = it.value();

                AudioAsset asset;
                asset.name = assetName;
                asset.id = nextID++;
                asset.path = assetData["path"];
                asset.loop = assetData["loop"];
                asset.group = assetData["group"];
                asset.spatial = assetData.value("spatial", false);
                asset.minDistance = assetData.value("minDistance", 1.0f);
                asset.maxDistance = assetData.value("maxDistance", 100.0f);
                asset.sound = nullptr;

                assetsByName[asset.name] = asset;
                idToName[asset.id] = asset.name;
            }
            };

        parseCategory("music");
        parseCategory("sfx");
        parseCategory("ui");
        parseCategory("ambience");

        // Parse banks
        if (manifest.contains("banks")) {
            const json& banksJson = manifest["banks"];
            for (auto it = banksJson.begin(); it != banksJson.end(); ++it) {
                const std::string& bankName = it.key();
                const json& assetList = it.value();

                std::vector<std::string> assets;
                for (const auto& assetName : assetList) {
                    assets.push_back(assetName.get<std::string>());
                }

                banks[bankName] = assets;
            }
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[AudioCache] Error parsing manifest: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief Shuts down the AudioCache and releases all loaded audio resources.
 */
void AudioCache::Shutdown() {
    for (auto& [name, asset] : assetsByName) {
        if (asset.sound) {
            asset.sound->release();
            asset.sound = nullptr;
        }
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
    auto it = assetsByName.find(name);
    if (it != assetsByName.end()) {
        return &it->second;
    }
    return nullptr;
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
    AudioAsset* asset = GetAsset(name);
    if (asset && asset->sound) {
        return asset->sound;
    }

    if (asset && !asset->sound) {
        if (LoadAsset(name)) {
            return asset->sound;
        }
    }

    return nullptr;
}

/**
 * @brief Checks if an audio asset has been loaded into memory.
 * @param name The name identifier of the audio asset.
 * @return bool True if the asset exists and its FMOD::Sound is loaded,
 *         false otherwise.
 */
bool AudioCache::IsLoaded(const std::string& name) {
    AudioAsset* asset = GetAsset(name);
    return (asset && asset->sound != nullptr);
}

/**
 * @brief Explicitly loads an audio asset into memory.
 * @param name The name identifier of the audio asset to load.
 * @return bool True if the asset was loaded successfully or was already
 *         loaded, false if the asset doesn't exist or loading failed.
 */
bool AudioCache::LoadAsset(const std::string& name) {
    AudioAsset* asset = GetAsset(name);
    if (!asset) {
        std::cerr << "[AudioCache] Asset not found: " << name << std::endl;
        return false;
    }

    if (asset->sound) {
        return true;
    }

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
    FMOD_MODE mode = asset.spatial ? FMOD_3D : FMOD_2D;

    if (asset.loop) mode |= FMOD_LOOP_NORMAL;
    else mode |= FMOD_LOOP_OFF;
    if (asset.spatial) mode |= FMOD_3D_LINEARROLLOFF;

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
    AudioAsset* asset = GetAsset(name);
    if (!asset) return;

    if (asset->sound) {
        asset->sound->release();
        asset->sound = nullptr;
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
    auto it = banks.find(bankName);
    if (it == banks.end()) {
        std::cerr << "[AudioCache] Bank not found: " << bankName << std::endl;
        return false;
    }

    const std::vector<std::string>& assets = it->second;
    bool allLoaded = true;

    for (const std::string& assetName : assets) {
        if (!LoadAsset(assetName)) {
            std::cerr << "[AudioCache] Failed to load asset in bank: " << assetName << std::endl;
            allLoaded = false;
        }
    }

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

    const std::vector<std::string>& assets = it->second;
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
    if (it != banks.end()) {
        return it->second;
    }
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
    return assetsByName;
}