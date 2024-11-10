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

    bool rightMouseButtonPressed = false;
    bool middleMouseButtonPressed = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

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
        const float cameraRotationSpeed = 5.0f;

        if (key == GLFW_KEY_S) {
            camera.moveForward(-cameraSpeed);
        }
        else if (key == GLFW_KEY_W) {
            camera.moveForward(cameraSpeed);
        }
        else if (key == GLFW_KEY_D) {
            camera.moveRight(cameraSpeed);
        }
        else if (key == GLFW_KEY_A) {
            camera.moveRight(-cameraSpeed);
        }
        else if (key == GLFW_KEY_Q) {
            camera.rotateAroundUp(cameraRotationSpeed);
        }
        else if (key == GLFW_KEY_E) {
            camera.rotateAroundUp(-cameraRotationSpeed);
        }
        else if (key == GLFW_KEY_Z) {
            camera.rotateAroundRight(cameraRotationSpeed);
        }
        else if (key == GLFW_KEY_X) {
            camera.rotateAroundRight(-cameraRotationSpeed);
        }
    }

    if (key == GLFW_KEY_ESCAPE)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

inline static void glfwErrorCallback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

inline static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        app->rightMouseButtonPressed = (action == GLFW_PRESS);
        glfwGetCursorPos(window, &app->lastMouseX, &app->lastMouseY);
    }
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        app->middleMouseButtonPressed = (action == GLFW_PRESS);
        glfwGetCursorPos(window, &app->lastMouseX, &app->lastMouseY);
    }
}

inline static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    VkRenderer& renderer = app->m_vulkanCtx;
    Camera& camera = renderer.getRendererCamera();

    if (app->rightMouseButtonPressed) {
        float xoffset = static_cast<float>(xpos - app->lastMouseX);
        float yoffset = static_cast<float>(app->lastMouseY - ypos);

        const float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        camera.rotateAroundUp(xoffset);
        camera.rotateAroundRight(yoffset);

        app->lastMouseX = xpos;
        app->lastMouseY = ypos;
    }else if (app->middleMouseButtonPressed) {
        float xoffset = static_cast<float>(xpos - app->lastMouseX);
        float yoffset = static_cast<float>(app->lastMouseY - ypos);

        const float moveSpeed = 0.01f;
        xoffset *= moveSpeed;
        yoffset *= moveSpeed;

        camera.moveRight(-xoffset);
        camera.moveUp(yoffset);

        app->lastMouseX = xpos;
        app->lastMouseY = ypos;
    }
}

inline static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    VkRenderer& renderer = app->m_vulkanCtx;
    Camera& camera = renderer.getRendererCamera();

    const float scrollSpeed = 0.5f;

    camera.moveForward(static_cast<float>(yoffset) * scrollSpeed);
}


#endif