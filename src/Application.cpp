#include "Application.h"

Application::Application() {
	m_renderer.init();
}

Application::~Application() {
	m_renderer.cleanup();
}

void Application::run() {
	m_renderer.mainLoop();
}