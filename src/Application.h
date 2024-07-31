#ifndef APPLICATION_H
#define APPLICATION_H

#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Application {
public:

	bool framebufferResized = false;

	Application();
	~Application();
	void run();

private:
	GLFWwindow* window;

	void initGlfw();
	//void initVulkanCtx();
	//void initImguiCtx();

	//void cleanUpImgui();
	//void cleanupVulkan();
	void cleanupGlfw();
};

// Static callback functions
static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void glfwErrorCallback(int error, const char* description);

#endif