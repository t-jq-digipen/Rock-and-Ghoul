/* ========================================================
 * @file    AudioCache.h
 * @brief   Centralized audio asset loading and caching system
 * ========================================================
 */
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "fmod.hpp"

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

enum class AudioGroup {
    MUSIC,
    SFX,
    UI,
    AMBIENCE
};

class AudioCache {
public:
    static bool Initialize(const std::string& manifestPath);
    static void Shutdown();

    static AudioAsset* GetAsset(const std::string& name);
    static AudioAsset* GetAssetByID(int id);

    static FMOD::Sound* GetSound(const std::string& name);
    static FMOD::Sound* GetSoundByID(int id);

    static bool IsLoaded(const std::string& name);
    static bool LoadAsset(const std::string& name);
    static void UnloadAsset(const std::string& name);

    static bool LoadBank(const std::string& bankName);
    static void UnloadBank(const std::string& bankName);
    static std::vector<std::string> GetBankAssets(const std::string& bankName);

private:
    static std::unordered_map<std::string, AudioAsset> assetsByName;
    static std::unordered_map<int, std::string> idToName;

    static bool ParseManifest(const std::string& manifestPath);
    static bool LoadAudioFile(AudioAsset& asset);

    static std::unordered_map<std::string, std::vector<std::string>> banks;
};