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

bool AudioCache::Initialize(const std::string& manifestPath) {
    std::cout << "[AudioCache] Initializing from: " << manifestPath << std::endl;

    if (!ParseManifest(manifestPath)) {
        std::cerr << "[AudioCache] Failed to parse manifest!" << std::endl;
        return false;
    }

    std::cout << "[AudioCache] Loaded " << assetsByName.size() << " audio assets" << std::endl;
    std::cout << "[AudioCache] Loaded " << banks.size() << " banks" << std::endl;
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

        // Helper to parse a category
        auto parseCategory = [&](const std::string& categoryName) {
            if (!manifest.contains(categoryName)) return;

            const json& category = manifest[categoryName];
            for (auto it = category.begin(); it != category.end(); ++it) {
                const std::string& assetName = it.key();
                const json& assetData = it.value();

                AudioAsset asset;
                asset.name = assetName;
                asset.id = assetData["id"];
                asset.path = assetData["path"];
                asset.loop = assetData["loop"];
                asset.group = assetData["group"];
                asset.spatial = assetData.value("spatial", false);
                asset.sound = nullptr;

                assetsByName[asset.name] = asset;
                idToName[asset.id] = asset.name;

                std::cout << "[AudioCache]   - " << categoryName << ": " << assetName
                    << " (ID: " << asset.id << ")" << std::endl;
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
                std::cout << "[AudioCache]   - Bank: " << bankName
                    << " (" << assets.size() << " assets)" << std::endl;
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
    std::cout << "[AudioCache] Shutting down..." << std::endl;

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
        std::cout << "[AudioCache] Auto-loading: " << name << std::endl;
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
        std::cout << "[AudioCache] Asset already loaded: " << name << std::endl;
        return true;
    }

    return LoadAudioFile(*asset);
}

bool AudioCache::LoadAudioFile(AudioAsset& asset) {
    std::cout << "[AudioCache] Loading: " << asset.name << " from " << asset.path << std::endl;

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
        std::cout << "[AudioCache] Unloading: " << name << std::endl;
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

    std::cout << "[AudioCache] Loading bank: " << bankName << std::endl;

    const std::vector<std::string>& assets = it->second;
    bool allLoaded = true;

    for (const std::string& assetName : assets) {
        if (!LoadAsset(assetName)) {
            std::cerr << "[AudioCache] Failed to load asset in bank: " << assetName << std::endl;
            allLoaded = false;
        }
    }

    if (allLoaded) {
        std::cout << "[AudioCache] Bank loaded successfully: " << bankName
            << " (" << assets.size() << " assets)" << std::endl;
    }

    return allLoaded;
}

void AudioCache::UnloadBank(const std::string& bankName) {
    auto it = banks.find(bankName);
    if (it == banks.end()) {
        std::cerr << "[AudioCache] Bank not found: " << bankName << std::endl;
        return;
    }

    std::cout << "[AudioCache] Unloading bank: " << bankName << std::endl;

    const std::vector<std::string>& assets = it->second;
    for (const std::string& assetName : assets) {
        UnloadAsset(assetName);
    }

    std::cout << "[AudioCache] Bank unloaded: " << bankName << std::endl;
}

std::vector<std::string> AudioCache::GetBankAssets(const std::string& bankName) {
    auto it = banks.find(bankName);
    if (it != banks.end()) {
        return it->second;
    }
    return {};
}