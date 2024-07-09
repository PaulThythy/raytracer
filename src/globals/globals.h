#ifndef GLOBALS_H
#define GLOBALS_H

#include <vector>
#include <string>

namespace Config {
	extern int SAMPLES;
	extern int INIT_WINDOW_WIDTH;
	extern int INIT_WINDOW_HEIGHT;

	void initialize();

	std::vector<char> readFile(const std::string& filename);
}

#endif