#ifndef GLOBALS_H
#define GLOBALS_H

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace Config {

    inline int SAMPLES = 1;
    inline uint32_t INIT_WINDOW_WIDTH = 1200;
	inline uint32_t INIT_WINDOW_HEIGHT = 1000;

    inline bool SHOW_DEMO_WINDOW = true;
    inline bool SHOW_ANOTHER_WINDOW = false;

    inline std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

}

#endif