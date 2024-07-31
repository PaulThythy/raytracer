#include "Application.h"
#include "globals/globals.h"

Application::Application() {
	initGlfw();
	//initVulkanCtx();
	//initImguiCtx();
}

Application::~Application() {
    std::cout << "Cleaning up..." << std::endl;
	
	//cleanUpImgui();
	//cleanupVulkan();
	cleanupGlfw();
}

void Application::run() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	//vkDeviceWaitIdle(logicalDevice);
}

void Application::initGlfw() {
	if (!glfwInit()) {
		throw std::runtime_error("Unable to initialize GLFW!");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(Config::INIT_WINDOW_WIDTH, Config::INIT_WINDOW_HEIGHT, "Raytracer", nullptr, nullptr);

	// Add a pointer that allows GLFW to reference our instance
	glfwSetWindowUserPointer(window, this);

	//add callbacks
	glfwSetKeyCallback(window, keyCallback);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	glfwSetErrorCallback(glfwErrorCallback);

	if (!window) {
		glfwTerminate();
		throw std::runtime_error("Unable to create window!");
	}
}

void Application::cleanupGlfw() {
	glfwDestroyWindow(window);
	glfwTerminate();
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

static void glfwErrorCallback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}