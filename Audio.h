#pragma once

#include <string>

namespace Audio {
    bool init();
    void playSound(const std::string& filePath);
    void cleanup();
}