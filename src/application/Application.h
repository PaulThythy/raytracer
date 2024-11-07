#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <stdexcept>

#include "vulkan/VkRenderer.h"
#include "globals/globals.h"

class Application
{
public:
    Application();
    ~Application();

    GLFWwindow *m_window;
    VkRenderer m_vulkanCtx;

    void run();

    bool framebufferResized = false;

private:
    void initGlfw();
    void initVulkanCtx(GLFWwindow *window);

    void cleanupVulkan();
    void cleanupGlfw();
};

inline static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    auto app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;

    Config::INIT_WINDOW_WIDTH = width;
    Config::INIT_WINDOW_HEIGHT = height;
}

inline static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    auto app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
    VkRenderer &renderer = app->m_vulkanCtx;
    Camera &camera = renderer.getRendererCamera();

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        const float cameraSpeed = 0.1f;

        /*if (key == GLFW_KEY_Z) {
            camera.m_position += cameraSpeed * camera.m_front;
        } else if (key == GLFW_KEY_S) {
            camera.m_position -= cameraSpeed * camera.m_front;
        } else if (key == GLFW_KEY_Q) {
            camera.m_position += glm::normalize(glm::cross(camera.m_front, camera.m_up)) * cameraSpeed;
        } else if (key == GLFW_KEY_D) {
            camera.m_position -= glm::normalize(glm::cross(camera.m_front, camera.m_up)) * cameraSpeed;
        }*/

        if (key == GLFW_KEY_Z)
        {
            camera.m_position.x += cameraSpeed;
        } else if (key == GLFW_KEY_S) {
            camera.m_position.x -= cameraSpeed;
        } else if (key == GLFW_KEY_Q) {
            
        } else if (key == GLFW_KEY_D) {
            
        }
    }

    if (key == GLFW_KEY_ESCAPE)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static void glfwErrorCallback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

#endif