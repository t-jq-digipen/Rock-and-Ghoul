#include "audio.h"
#include "AudioSystem.h"
#include "CoreEngine.h"
#include <iostream>

// Static members
FMOD::System* AudioSystem::mSystem = nullptr;

// ===== ADDED: Button sound storage =====
std::unordered_map<AudioSystem::ButtonType, FMOD::Sound*> AudioSystem::buttonSounds;
// ===== END ADDED =====

bool AudioSystem::Init() {
    FMOD_RESULT result;

    result = FMOD::System_Create(&mSystem);
    if (result != FMOD_OK) {
        std::cerr << "FMOD Error: Failed to create system (" << result << ")" << std::endl;
        return false;
    }

    result = mSystem->init(32, FMOD_INIT_NORMAL, nullptr);
    if (result != FMOD_OK) {
        std::cerr << "FMOD Error: Failed to initialize system (" << result << ")" << std::endl;
        return false;
    }

    // ===== ADDED: Load default button sounds =====
    LoadDefaultButtonSounds();
    // ===== END ADDED =====

    std::cout << "FMOD Audio initialized successfully!" << std::endl;
    return true;
}

// ===== ADDED: Flexible Button SFX Implementation =====
void AudioSystem::LoadDefaultButtonSounds() {
    // Register default button sounds based on your ButtonType enum
    RegisterButtonSound(ButtonType::PLAY, "../Assets/music/playclick.mp3");
    RegisterButtonSound(ButtonType::QUIT, "../Assets/music/quitclick.mp3");

    std::cout << "Default button sounds loaded!" << std::endl;
}

void AudioSystem::PlayButtonSound(ButtonType type) {
    auto it = buttonSounds.find(type);
    if (it != buttonSounds.end() && it->second) {
        PlaySound(it->second);
        std::cout << "Playing button sound for type: " << static_cast<int>(type) << std::endl;
    }
    else {
        std::cerr << "Warning: Button sound not found for type " << static_cast<int>(type) << std::endl;

        // Fallback: try to play PLAY sound if the requested type isn't found
        if (type != ButtonType::PLAY) {
            auto playIt = buttonSounds.find(ButtonType::PLAY);
            if (playIt != buttonSounds.end() && playIt->second) {
                PlaySound(playIt->second);
            }
        }
    }
}

void AudioSystem::RegisterButtonSound(ButtonType type, const std::string& soundPath) {
    FMOD::Sound* sound = LoadSound(soundPath, false);
    if (sound) {
        buttonSounds[type] = sound;
        std::cout << "Registered button sound for type " << static_cast<int>(type)
            << " from: " << soundPath << std::endl;
    }
    else {
        std::cerr << "Failed to load button sound: " << soundPath << std::endl;
    }
}

void AudioSystem::RegisterButtonSound(ButtonType type, FMOD::Sound* sound) {
    if (sound) {
        buttonSounds[type] = sound;
        std::cout << "Registered button sound for type " << static_cast<int>(type) << std::endl;
    }
}
// ===== END ADDED =====

void AudioSystem::Close() {
    // ===== ADDED: Clean up button sounds =====
    for (auto& pair : buttonSounds) {
        if (pair.second) {
            pair.second->release();
        }
    }
    buttonSounds.clear();
    // ===== END ADDED =====

    if (mSystem) {
        mSystem->close();
        mSystem->release();
        mSystem = nullptr;
    }
}

void AudioSystem::Update() {
    if (mSystem) {
        mSystem->update();
    }
}

FMOD::Sound* AudioSystem::LoadSound(const std::string& filename, bool loop) {
    if (!mSystem) return nullptr;

    FMOD::Sound* sound = nullptr;
    FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_DEFAULT;

    FMOD_RESULT result = mSystem->createSound(filename.c_str(), mode, nullptr, &sound);
    if (result != FMOD_OK) {
        std::cerr << "FMOD Error: Failed to load sound (" << filename << ")" << std::endl;
        return nullptr;
    }

    return sound;
}

void AudioSystem::PlaySound(FMOD::Sound* sound) {
    if (!mSystem || !sound) return;
    FMOD::Channel* channel = nullptr;
    mSystem->playSound(sound, nullptr, false, &channel);
}

void AudioSystem::StopAll() {
    if (!mSystem) return;
    mSystem->close();
    mSystem->init(32, FMOD_INIT_NORMAL, nullptr);
}

FMOD::Channel* AudioSystem::PlaySoundGetChannel(FMOD::Sound* sound, bool paused) {
    if (!mSystem || !sound) return nullptr;
    FMOD::Channel* channel = nullptr;
    FMOD_RESULT result = mSystem->playSound(sound, nullptr, paused, &channel);
    if (result != FMOD_OK) {
        std::cerr << "FMOD Error: Failed to play sound with channel\n";
        return nullptr;
    }
    return channel;
}

void AudioSystem::SetChannelPosition(FMOD::Channel* channel, unsigned int ms) {
    if (channel) {
        channel->setPosition(ms, FMOD_TIMEUNIT_MS);
    }
}

unsigned int AudioSystem::GetChannelPosition(FMOD::Channel* channel) {
    unsigned int pos = 0;
    if (channel) {
        channel->getPosition(&pos, FMOD_TIMEUNIT_MS);
    }
    return pos;
}

void AudioSystem::SetChannelPaused(FMOD::Channel* channel, bool paused) {
    if (channel) {
        channel->setPaused(paused);
    }
}

// ============================================================================
// AudioManager Implementation
// ============================================================================

void AudioManager::Initialize() {
    AudioSystem::Init();

    // preload music tracks
    mainMenuMusic = Load("../Assets/music/sniffmyBalls.wav", true);
    level1Music = Load("../Assets/music/sniffmyBallsAgain.wav", true);

    // ===== REMOVED: No need to manually load button sounds =====
    // Button sounds are now automatically loaded in AudioSystem::Init()
    // ===== END REMOVED =====

    // start main menu music
    if (mainMenuMusic) {
        PlayMusic(mainMenuMusic);
        std::cout << "[AudioManager] Main menu music started\n";
    }

    lastState = GameState::GS_MAIN_MENU;
}

void AudioManager::Update(float dt) {
    AudioSystem::Update();

    UpdateCrossfade(dt);

    // STATE BASED MUSIC CHANGES
    GameState currentState = GSM_CurrentState();
    HandleMusicForState(currentState, lastState);
    lastState = currentState;
}

void AudioManager::UpdateCrossfade(float dt) {
    if (!crossfadeActive) return;

    crossfadeTime += dt;
    float t = crossfadeTime / crossfadeDuration;
    if (t > 1.0f) t = 1.0f;

    if (currentChannel)
        currentChannel->setVolume(musicVol * (1.0f - t));  // Fade from musicVol to 0
    if (nextChannel)
        nextChannel->setVolume(musicVol * t);              // Fade from 0 to musicVol

    if (t >= 1.0f) {
        if (currentChannel) currentChannel->stop();
        currentChannel = nextChannel;
        nextChannel = nullptr;
        crossfadeActive = false;
        std::cout << "[AudioManager] Crossfade completed\n";
    }
}

void AudioManager::HandleMusicForState(GameState currentState, GameState previousState) {
    // DETECT TRANSITION TO LEVEL 1!
    if (currentState == GameState::GS_LEVEL1 && previousState != GameState::GS_LEVEL1) {
        std::cout << "[AudioManager] Level 1 started - crossfading music...\n";
        if (level1Music) {
            CrossfadeTo(level1Music, 4.0f);
        }
        else {
            std::cerr << "[AudioManager] Error: Level 1 music not loaded!\n";
        }
    }
}

void AudioManager::Shutdown() {
    AudioSystem::Close();
}

FMOD::Sound* AudioManager::Load(const std::string& path, bool loop) {
    return AudioSystem::LoadSound(path, loop);
}

void AudioManager::Play(FMOD::Sound* sound) {
    AudioSystem::PlaySound(sound);
}

FMOD::Channel* AudioManager::PlayGetChannel(FMOD::Sound* sound, bool paused) {
    return AudioSystem::PlaySoundGetChannel(sound, paused);
}

void AudioManager::SetPosition(FMOD::Channel* channel, unsigned int ms) {
    if (channel) {
        channel->setPosition(ms, FMOD_TIMEUNIT_MS);
    }
}

unsigned int AudioManager::GetPosition(FMOD::Channel* channel) {
    unsigned int pos = 0;
    if (channel) {
        channel->getPosition(&pos, FMOD_TIMEUNIT_MS);
    }
    return pos;
}

void AudioManager::SetPaused(FMOD::Channel* channel, bool paused) {
    if (channel) {
        channel->setPaused(paused);
    }
}

void AudioManager::PlayMusic(FMOD::Sound* track, bool paused) {
    currentChannel = PlayGetChannel(track, paused);
    if (currentChannel && !paused)
        currentChannel->setVolume(musicVol);
}

void AudioManager::CrossfadeTo(FMOD::Sound* newTrack, float duration) {
    if (!newTrack) return;

    nextChannel = PlayGetChannel(newTrack, true);
    if (!nextChannel) return;

    nextChannel->setVolume(0.0f);
    nextChannel->setPaused(false);

    crossfadeTime = 0.0f;
    crossfadeDuration = duration;
    crossfadeActive = true;

    std::cout << "[AudioManager] Starting crossfade (" << duration << "s)\n";
}