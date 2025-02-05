#pragma once

#include <string>
#include "miniaudio.h"
#include <map>
#include <vector>

class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();

    bool playSound3D(const std::string& name, float soundX, float soundY, float soundZ, float listX, float listY, float listZ, float listXDir, float listYDir, float listZDir);
    void cleanFinishedSounds();

private:
    ma_engine engine;
    std::map<std::string, ma_sound*> soundBank;
    std::vector<ma_sound*> activeSounds;
    bool initialized;

    bool loadSound(const std::string& name, ma_sound* snd);
    void loadSystemSounds();
    void setListenerPosition(float listX, float listY, float listZ, float listXDir, float listYDir, float listZDir);
    ma_sound* getSoundFromBank(const std::string& name);
};
