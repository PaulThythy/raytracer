#ifndef GLOBALS_H
#define GLOBALS_H

#include <vector>
#include <string>

namespace Config {
	extern int SAMPLES;
	extern uint32_t INIT_WINDOW_WIDTH;
	extern uint32_t INIT_WINDOW_HEIGHT;

	std::vector<char> readFile(const std::string& filename);
}

#endif