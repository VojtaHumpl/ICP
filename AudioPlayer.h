#pragma once

#include <string>
#include "miniaudio.h"

class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();

    bool playSound(const std::string& filePath);
    void cleanup();

private:
    ma_engine engine;
    bool initialized;
};
