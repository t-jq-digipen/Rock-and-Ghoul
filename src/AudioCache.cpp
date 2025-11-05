/* ========================================================
 * @file    AudioCache.cpp
 * @brief   Implementation of AudioCache system with Bank Loading
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

bool AudioCache::Initialize(const std::string& manifestPath) {
    if (!ParseManifest(manifestPath)) {
        std::cerr << "[AudioCache] Failed to parse manifest!" << std::endl;
        return false;
    }

    return true;
}

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

AudioAsset* AudioCache::GetAsset(const std::string& name) {
    auto it = assetsByName.find(name);
    if (it != assetsByName.end()) {
        return &it->second;
    }
    return nullptr;
}

AudioAsset* AudioCache::GetAssetByID(int id) {
    auto it = idToName.find(id);
    if (it != idToName.end()) {
        return GetAsset(it->second);
    }
    return nullptr;
}

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

FMOD::Sound* AudioCache::GetSoundByID(int id) {
    auto it = idToName.find(id);
    if (it != idToName.end()) {
        return GetSound(it->second);
    }
    return nullptr;
}

bool AudioCache::IsLoaded(const std::string& name) {
    AudioAsset* asset = GetAsset(name);
    return (asset && asset->sound != nullptr);
}

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

std::vector<std::string> AudioCache::GetBankAssets(const std::string& bankName) {
    auto it = banks.find(bankName);
    if (it != banks.end()) {
        return it->second;
    }
    return {};
}

const std::unordered_map<std::string, AudioAsset>& AudioCache::GetAllAssets() {
    return assetsByName;
}