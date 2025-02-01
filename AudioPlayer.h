#pragma once

#include <string>
#include "miniaudio.h"

class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();

    bool playSound(const std::string& filePath);
    bool playMissileExplosionSound();
    bool playMissileLaunchSound();
    bool playSneezeSound();
    void cleanup();

private:
    ma_engine engine;
    bool initialized;
};
