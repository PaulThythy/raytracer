#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <stdexcept>

#include "vulkan/VulkanContext.h"
#include "globals/globals.h"

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

inline static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;

    Config::INIT_WINDOW_WIDTH = width;
    Config::INIT_WINDOW_HEIGHT = height;
}

inline static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static void glfwErrorCallback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

#endif