#include "AudioPlayer.h"
#include <iostream>
#include <stdexcept>

AudioPlayer::AudioPlayer() : initialized(false) {
    if (ma_engine_init(nullptr, &engine) != MA_SUCCESS) {
        throw std::runtime_error("Failed to initialize audio engine.");
    }
    initialized = true;
}

AudioPlayer::~AudioPlayer() {
    cleanup();
}

bool AudioPlayer::playSound(const std::string& filePath) {
    if (!initialized) {
        std::cerr << "AudioPlayer not initialized." << std::endl;
        return false;
    }
    if (ma_engine_play_sound(&engine, filePath.c_str(), nullptr) != MA_SUCCESS) {
        std::cerr << "Failed to play sound: " << filePath << std::endl;
        return false;
    }
    return true;
}

bool AudioPlayer::playMissileExplosionSound() {
    return playSound("resources/missile_explosion_sound.wav");
}

bool AudioPlayer::playMissileLaunchSound() {
    return playSound("resources/missile_launch_sound.wav");
}

bool AudioPlayer::playSneezeSound() {
    return playSound("resources/sneeze_sound.wav");
}

void AudioPlayer::cleanup() {
    if (initialized) {
        ma_engine_uninit(&engine);
        initialized = false;
    }
}
