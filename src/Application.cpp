#include "Application.h"
#include "globals/globals.h"

Application::Application() {
	initGlfw();
	initVulkanCtx(m_window);
}

Application::~Application() {
	std::cout << "Cleaning up..." << std::endl;

	cleanupVulkan();
	cleanupGlfw();
}

Application::~Application() {
	std::cout << "Cleaning up..." << std::endl;

	cleanupVulkan();
	cleanupGlfw();
}

void Application::run() {
	while (!glfwWindowShouldClose(m_window)) {
		glfwPollEvents();
	}

	//vkDeviceWaitIdle(logicalDevice);
}

void Application::initGlfw() {
	if (!glfwInit()) {
		throw std::runtime_error("GLFW: Unable to initialize GLFW!");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	if (!glfwVulkanSupported()) {
		throw std::runtime_error("GLFW: Vulkan not supported!");
	}

	m_window = glfwCreateWindow(Config::INIT_WINDOW_WIDTH, Config::INIT_WINDOW_HEIGHT, "Raytracer", nullptr, nullptr);

	// Add a pointer that allows GLFW to reference our instance
	glfwSetWindowUserPointer(m_window, this);

	//add callbacks
	glfwSetKeyCallback(m_window, keyCallback);
	glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
	glfwSetErrorCallback(glfwErrorCallback);

	if (!m_window) {
		glfwTerminate();
		throw std::runtime_error("GLFW: Unable to create window!");
	}
}

void Application::initVulkanCtx(GLFWwindow* window) { m_vulkanCtx.initVulkan(window); }

void Application::cleanupVulkan() {

}

void Application::cleanupGlfw() {
	glfwDestroyWindow(m_window);
  glfwTerminate();
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
  app->framebufferResized = true;

  Config::INIT_WINDOW_WIDTH = width;
  Config::INIT_WINDOW_HEIGHT = height;
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