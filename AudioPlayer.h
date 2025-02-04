#pragma once

#include <string>
#include "miniaudio.h"
#include <vector>

class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();

    bool playSound3D(ma_sound* sound);
    bool playMissileExplosionSound(float x, float y, float z);
    bool playMissileLaunchSound(float x, float y, float z);
    bool playSneezeSound(float x, float y, float z);
    void setListenerPosition(float x_pos, float y_pos, float z_pos, float x_dir, float y_dir, float z_dir);
    ma_sound* loadSound(const std::string& filePath);
    void update();
    void cleanup();

private:
    ma_engine engine;
    bool initialized;
    std::vector<ma_sound*> activeSounds;
};
