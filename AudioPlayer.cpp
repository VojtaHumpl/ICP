#define MINIAUDIO_IMPLEMENTATION
#include "MiniAudio.h"
#include "AudioPlayer.h"
#include <iostream>
#include <stdexcept>


AudioPlayer::AudioPlayer() : initialized(false) {
    // config audio engine
    ma_engine_config config = ma_engine_config_init();
    config.listenerCount = 1;  // one listener (player)
    config.channels = 1;      // mono sounds are required for 3D spatialization.

    if (ma_engine_init(&config, &engine) != MA_SUCCESS) {
        throw std::runtime_error("Failed to initialize audio engine.");
    }
    initialized = true;
}

ma_sound* AudioPlayer::loadSound(const std::string& filePath)
{
    // check if initialized
    if (!initialized) {
        std::cerr << "AudioPlayer not initialized." << std::endl;
        return nullptr;
    }

    // load sound from filepath
    ma_sound* sound = new ma_sound;
    if (ma_sound_init_from_file(&engine, filePath.c_str(), MA_SOUND_FLAG_ASYNC, nullptr, nullptr, sound) != MA_SUCCESS) {
        std::cerr << "Failed to load sound: " << filePath << std::endl;
        delete sound;
        return nullptr;
    }

    return sound;
}

bool AudioPlayer::playSound3D(ma_sound* sound)
{
    // check if initialized
    if (!initialized || sound == nullptr) {
        std::cerr << "AudioPlayer not initialized or sound is null." << std::endl;
        return false;
    }

    // play sound
    if (ma_sound_start(sound) != MA_SUCCESS) {
        ma_sound_uninit(sound);
        delete sound;
        return false;
    }

    // keep track so we can uninit later
    activeSounds.push_back(sound);

    return true;
}

bool AudioPlayer::playMissileExplosionSound(float x, float y, float z) {
    ma_sound* s = loadSound("resources/missile_explosion_sound.wav");
    if (!s) return false;

    ma_sound_set_position(s, x, y, z);
    ma_sound_set_min_distance(s, 0.1f);
    ma_sound_set_max_distance(s, 1000.0f);
    ma_sound_set_volume(s, 1.0f);

    return playSound3D(s);
}

bool AudioPlayer::playMissileLaunchSound(float x, float y, float z) {
    ma_sound* s = loadSound("resources/missile_launch_sound.wav");
    if (!s) return false;

    ma_sound_set_position(s, x, y, z);
    ma_sound_set_min_distance(s, 0.1f);
    ma_sound_set_max_distance(s, 1000.0f);
    ma_sound_set_volume(s, 1.0f);

    return playSound3D(s);
}

bool AudioPlayer::playSneezeSound(float x, float y, float z) {
    ma_sound* s = loadSound("resources/sneeze_sound.wav");
    if (!s) return false;

    ma_sound_set_position(s, x, y, z);
    ma_sound_set_min_distance(s, 0.1f);
    ma_sound_set_max_distance(s, 1000.0f);
    ma_sound_set_volume(s, 1.0f);

    return playSound3D(s);
}

void AudioPlayer::setListenerPosition(float x_pos, float y_pos, float z_pos, float x_dir, float y_dir, float z_dir) {
    if (initialized) {
        ma_engine_listener_set_position(&engine, 0, x_pos, y_pos, z_pos); // set listener position
        ma_engine_listener_set_direction(&engine, 0, x_dir, y_dir, z_dir);  // facing
    }
}

void AudioPlayer::update()
{
    // iterate and remove any sounds that have reached the end
    for (auto it = activeSounds.begin(); it != activeSounds.end(); )
    {
        ma_sound* snd = *it;
        if (ma_sound_at_end(snd)) {
            ma_sound_uninit(snd);
            delete snd;
            it = activeSounds.erase(it);
        }
        else {
            ++it;
        }
    }
}

void AudioPlayer::cleanup() {
    if (initialized) {
        ma_engine_uninit(&engine);
        initialized = false;
    }
}

AudioPlayer::~AudioPlayer() {
    // uninit remaining sounds
    for (ma_sound* snd : activeSounds) {
        ma_sound_uninit(snd);
        delete snd;
    }

    // uninit engine
    cleanup();
}
