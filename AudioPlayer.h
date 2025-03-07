#pragma once

#include <string>
#include "miniaudio.h"
#include <map>
#include <vector>

struct ActiveSound {
    std::string name;
    ma_sound* soundPtr;
};

class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();

    bool playSound3D(const std::string& name, float soundX, float soundY, float soundZ, float listX, float listY, float listZ, float listXDir, float listYDir, float listZDir);
    void cleanFinishedSounds();
    bool playSound3DOnce(const std::string& name, float soundX, float soundY, float soundZ, float listX, float listY, float listZ, float listXDir, float listYDir, float listZDir);
    void setVolume(float volume);

private:
    ma_engine engine;
    std::map<std::string, ma_sound*> soundBank;
    std::vector<ActiveSound> activeSounds;
    bool initialized;
    float volume = 1.0;

    bool loadSound(const std::string& name, ma_sound* snd);
    void loadSystemSounds();
    void setListenerPosition(float listX, float listY, float listZ, float listXDir, float listYDir, float listZDir);
    ma_sound* getSoundFromBank(const std::string& name);
	bool isSoundActive(const std::string& name);
};

extern AudioPlayer gAudioPlayer;
