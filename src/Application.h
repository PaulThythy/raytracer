#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <stdexcept>

#include "vulkan/VulkanContext.h"

class Application {
public:
    Application();
    ~Application();

    GLFWwindow* m_window;

    void run();

    bool framebufferResized = false;
private:
    VulkanContext m_vulkanCtx;

    void initGlfw();
    void initVulkanCtx(GLFWwindow* window);

    void cleanupVulkan();
    void cleanupGlfw();
};

static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void glfwErrorCallback(int error, const char* description);

#endif