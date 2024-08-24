#ifndef GLOBALS_H
#define GLOBALS_H

#include <vector>
#include <string>

namespace Config {
	extern int SAMPLES;
	extern uint32_t INIT_WINDOW_WIDTH;
	extern uint32_t INIT_WINDOW_HEIGHT;

	extern bool SHOW_DEMO_WINDOW;
	extern bool SHOW_ANOTHER_WINDOW;

	std::vector<char> readFile(const std::string& filename);
}

#endif