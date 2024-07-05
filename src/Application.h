#ifndef APPLICATION_H
#define APPLICATION_H

#include "Renderer.h"
#include "Shader.h"

class Application {
private:
	Renderer m_renderer;

	Shader m_shader;
public:
	Application();
	~Application();

	void run();
};

#endif