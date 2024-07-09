#include "globals.h"

#include <iostream>
#include <fstream>
#include <stdexcept>

namespace Config {

    int SAMPLES = 1;
    int INIT_WINDOW_WIDTH = 1920;
	int INIT_WINDOW_HEIGHT = 1080;

    void initialize() {}

    std::vector<char> readFile(const std::string& filename) {
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