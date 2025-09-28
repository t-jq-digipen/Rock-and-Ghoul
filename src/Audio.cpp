#include "audio.h"
#include <iostream>

// Static members
FMOD::System* AudioSystem::mSystem = nullptr;

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

    std::cout << "FMOD Audio initialized successfully!" << std::endl;
    return true;
}

void AudioSystem::Close() {
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
    mSystem->close(); // stop all
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
