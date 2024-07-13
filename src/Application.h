#ifndef APPLICATION_H
#define APPLICATION_H

#include "Renderer.h"

class Application {
private:
	Renderer m_renderer;
public:
	Application();
	~Application();

	void run();
};

#endif