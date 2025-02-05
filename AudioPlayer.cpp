#define MINIAUDIO_IMPLEMENTATION
#include "MiniAudio.h"
#include "AudioPlayer.h"
#include <iostream>
#include <stdexcept>

/// <summary>
/// AudioPlayer constructor.
/// </summary>
AudioPlayer::AudioPlayer() : initialized(false) {
    // config audio engine
    ma_engine_config config = ma_engine_config_init();
    config.listenerCount = 1;  // one listener (player)
    config.channels = 1;      // mono sounds are required for 3D

    // try to init engine
    if (ma_engine_init(&config, &engine) != MA_SUCCESS) {
        throw std::runtime_error("Failed to initialize audio engine.");
    }

    // set initialized
    initialized = true;

    // load system sounds
    loadSystemSounds();

}

/// <summary>
/// AudioPlayer desctructor.
/// </summary>
AudioPlayer::~AudioPlayer() {
    // uninitialize and free all loaded sounds.
    for (auto& kv : soundBank) {
        ma_sound_uninit(kv.second);
        delete kv.second;
    }

    // clean rest of the active sounds
    for (auto it = activeSounds.begin(); it != activeSounds.end();) {
            ma_sound_uninit(*it);
            delete* it;
            it = activeSounds.erase(it);
    }

    // uninitialize engine
    ma_engine_uninit(&engine);
}

/// <summary>
/// Play sound by its name from the sound bank. Also define sound position and listener position + direction.
/// </summary>
bool AudioPlayer::playSound3D(const std::string& name, float soundX, float soundY, float soundZ, float listX, float listY, float listZ, float listXDir, float listYDir, float listZDir) {
    // check if initialized
    if (!initialized) return false;

    //// get sound by name from the bank
    //auto it = soundBank.find(name);
    //if (it == soundBank.end()) {
    //    std::cerr << "Sound '" << name << "' not found." << std::endl;
    //    return false;
    //}
    //ma_sound* snd = it->second;

    // get sound copy from the bank
    ma_sound* snd = getSoundFromBank(name);

    // reset playback to beginning
    ma_sound_seek_to_pcm_frame(snd, 0);

    // set the sound properties
    ma_sound_set_position(snd, soundX, soundY, soundZ);

    // set listener position
    setListenerPosition(listX, listY, listZ, listXDir, listYDir, listZDir);

    // play the sound
    if (ma_sound_start(snd) != MA_SUCCESS) {
        std::cerr << "Failed to play sound: " << name << std::endl;
        return false;
    }

    // add sound to active sounds
    activeSounds.push_back(snd);

    std::cerr << name << " sound playing." << std::endl;

    return true;
}

/// <summary>
/// Get copy of the sound from the sound bank
/// </summary>
ma_sound* AudioPlayer::getSoundFromBank(const std::string& name) {
    auto it = soundBank.find(name);
    if (it == soundBank.end()) {
        std::cerr << "Sound '" << name << "' not found in sound bank." << std::endl;
        return nullptr;
    }

    ma_sound* original = it->second;
    ma_sound* soundCopy = new ma_sound;
    if (ma_sound_init_copy(&engine, original, MA_SOUND_FLAG_ASYNC, nullptr, soundCopy) != MA_SUCCESS) {
        std::cerr << "Failed to copy sound: " << name << std::endl;
        delete soundCopy;
        return nullptr;
    }
    return soundCopy;
}

/// <summary>
/// Load sound and its key name into the sound bank.
/// </summary>
bool AudioPlayer::loadSound(const std::string& name, ma_sound* snd) {
    // check if initialized
    if (!initialized) return false;

    // check if sound is already in bank
    if (soundBank.find(name) != soundBank.end()) {
        std::cerr << "Sound '" << name << "' already loaded." << std::endl;
        return false;
    }

    soundBank[name] = snd;
    return true;
}

/// <summary>
/// Load defined system sounds.
/// </summary>
void AudioPlayer::loadSystemSounds() {
    // --------------------------------- explosion ---------------------------------
    ma_sound* explosion = new ma_sound;
    if (ma_sound_init_from_file(&engine, "resources/missile_explosion_sound.wav", MA_SOUND_FLAG_ASYNC, nullptr, nullptr, explosion) != MA_SUCCESS) {
        std::cerr << "Failed to load missile explosion sound." << std::endl;
        delete explosion;
    }
    else {
        // set sound parameters
        ma_sound_set_min_distance(explosion, 0.5f);
        ma_sound_set_max_distance(explosion, 200.0f);
        ma_sound_set_volume(explosion, 2.0f);

        // load the sound into the bank
        loadSound("EXPLOSION", explosion);
    }

    // --------------------------------- launch ---------------------------------
    ma_sound* launch = new ma_sound;
    if (ma_sound_init_from_file(&engine, "resources/missile_launch_sound.wav", MA_SOUND_FLAG_ASYNC, nullptr, nullptr, launch) != MA_SUCCESS) {
        std::cerr << "Failed to load missile launch sound." << std::endl;
        delete launch;
    }
    else {
        // set sound parameters
        ma_sound_set_min_distance(launch, 0.5f);
        ma_sound_set_max_distance(launch, 100.0f);
        ma_sound_set_volume(launch, 2.0f);

        // load the sound into the bank
        loadSound("LAUNCH", launch);
    }

    // --------------------------------- sneeze ---------------------------------
    ma_sound* sneeze = new ma_sound;
    if (ma_sound_init_from_file(&engine, "resources/sneeze_sound.wav", MA_SOUND_FLAG_ASYNC, nullptr, nullptr, sneeze) != MA_SUCCESS) {
        std::cerr << "Failed to load sneeze sound." << std::endl;
        delete sneeze;
    }
    else {
        // set sound parameters
        ma_sound_set_min_distance(sneeze, 0.5f);
        ma_sound_set_max_distance(sneeze, 30.0);
        ma_sound_set_volume(sneeze, 1.0f);

        // load the sound into the bank
        loadSound("SNEEZE", sneeze);
    }
}

/// <summary>
/// Set listener position and direction.
/// </summary>
void AudioPlayer::setListenerPosition(float listX, float listY, float listZ, float listXDir, float listYDir, float listZDir) {
    // check if initialized
    if (!initialized) {
        std::cerr << "Audio engine not initialized." << std::endl;
        return;
    }

    // set listener position, direction
    ma_engine_listener_set_position(&engine, 0, listX, listY, listZ);
    ma_engine_listener_set_direction(&engine, 0, listXDir, listYDir, listZDir);
}

/// <summary>
/// Clean finished sounds.
/// </summary>
void AudioPlayer::cleanFinishedSounds() {
    for (auto it = activeSounds.begin(); it != activeSounds.end();) {
        if (!ma_sound_is_playing(*it)) {  // if sound finished playing
            ma_sound_uninit(*it);
            delete* it;
            it = activeSounds.erase(it);
        }
        else {
            ++it;
        }
    }
}