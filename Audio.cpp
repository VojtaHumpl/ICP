#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "audio.h"
#include <iostream>

namespace Audio {
    static ma_engine engine;

    bool init() {
        if (ma_engine_init(nullptr, &engine) != MA_SUCCESS) {
            std::cerr << "Failed to initialize audio engine." << std::endl;
            return false;
        }
        return true;
    }

    void playSound(const std::string& filePath) {
        if (ma_engine_play_sound(&engine, filePath.c_str(), nullptr) != MA_SUCCESS) {
            std::cerr << "Failed to play sound: " << filePath << std::endl;
        }
    }

    void cleanup() {
        ma_engine_uninit(&engine);
    }
}
