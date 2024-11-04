#include "Application.h"

Application::Application() {
	initGlfw();
	initVulkanCtx(m_window);
}

Application::~Application() {
	std::cout << "Cleaning up..." << std::endl;

	cleanupVulkan();
	cleanupGlfw();
}

void Application::run() {
	while (!glfwWindowShouldClose(m_window)) {
		glfwPollEvents();

		m_vulkanCtx.mainLoop(m_window);
	}
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
	m_vulkanCtx.cleanupVulkan();
}

void Application::cleanupGlfw() {
	glfwDestroyWindow(m_window);
	glfwTerminate();
}