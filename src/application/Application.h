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
        } else if (key == GLFW_KEY_W) {
            camera.moveForward(cameraSpeed);
        } else if (key == GLFW_KEY_D) {
            camera.moveRight(cameraSpeed);
        } else if (key == GLFW_KEY_A) {
            camera.moveRight(-cameraSpeed);
        } else if (key == GLFW_KEY_Q) {
            camera.rotateAroundUp(cameraRotationSpeed);
        } else if (key == GLFW_KEY_E) {
            camera.rotateAroundUp(-cameraRotationSpeed);
        } else if (key == GLFW_KEY_Z) {
            camera.rotateAroundRight(cameraRotationSpeed);
        } else if (key == GLFW_KEY_X) {
            camera.rotateAroundRight(-cameraRotationSpeed);
        }

        std::cout << "Version in keyCallback :" << std::endl;
        std::cout << "position : (" << camera.m_cameraUBO.m_position.x << ", " << camera.m_cameraUBO.m_position.y << ", " << camera.m_cameraUBO.m_position.z << ")" << std::endl;
        std::cout << "lookAt : (" << camera.m_cameraUBO.m_lookAt.x << ", " << camera.m_cameraUBO.m_lookAt.y << ", " << camera.m_cameraUBO.m_lookAt.z << ")" << std::endl;
        std::cout << "front : (" << camera.m_cameraUBO.m_front.x << ", " << camera.m_cameraUBO.m_front.y << ", " << camera.m_cameraUBO.m_front.z << ")" << std::endl;
        std::cout << "up : (" << camera.m_cameraUBO.m_up.x << ", " << camera.m_cameraUBO.m_up.y << ", " << camera.m_cameraUBO.m_up.z << ")" << std::endl;
        std::cout << "right : (" << camera.m_cameraUBO.m_right.x << ", " << camera.m_cameraUBO.m_right.y << ", " << camera.m_cameraUBO.m_right.z << ")" << std::endl;
        std::cout << "worldUp : (" << camera.m_cameraUBO.m_worldUp.x << ", " << camera.m_cameraUBO.m_worldUp.y << ", " << camera.m_cameraUBO.m_worldUp.z << ")" << std::endl;
        std::cout << "fov : " << camera.m_cameraUBO.m_fov << std::endl;
        std::cout << "aspectRatio : " << camera.m_cameraUBO.m_aspectRatio << std::endl;
        std::cout << "nearPlane : " << camera.m_cameraUBO.m_nearPlane << std::endl;
        std::cout << "farPlane : " << camera.m_cameraUBO.m_farPlane << std::endl;
        std::cout << std::endl;
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