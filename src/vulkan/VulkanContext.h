#ifndef VULKAN_CONTEXT_H
#define VULKAN_CONTEXT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <stdexcept>
#include <array>
#include <iostream>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <set>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "../globals/globals.h"

class VulkanContext {
public:
	void initVulkan(GLFWwindow* window);
	void mainLoop(GLFWwindow* window);
	void cleanupVulkan();

private:
	VkInstance m_instance;
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	VkSurfaceKHR m_surface;
	VkDescriptorPool m_descriptorPool;
	VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;
	ImGui_ImplVulkanH_Window* m_wd;
	VkAllocationCallbacks* m_allocator = nullptr;
	VkDebugReportCallbackEXT m_debugReport = VK_NULL_HANDLE;
	VkDevice m_logicalDevice;
	VkQueue m_queue;
	uint32_t m_queueFamily = (uint32_t)-1;
	ImGui_ImplVulkanH_Window m_mainWindowData;

	ImGuiIO* m_io;

	int m_minImageCount = 2;
	bool m_swapChainRebuild = false;


	struct UniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;

		/*static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			return attributeDescriptions;
		}*/
	};

	const std::vector<Vertex> vertices = {
		{{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}}, // Vertex 0: Bottom-left corner (Red)
		{{ 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}}, // Vertex 1: Bottom-right corner (Green)
		{{ 1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}}, // Vertex 2: Top-right corner (Blue)
		{{-1.0f,  1.0f}, {1.0f, 1.0f, 1.0f}}  // Vertex 3: Top-left corner (White)
	};

	const std::vector<uint16_t> indices = {
		0, 1, 2,  // First triangle
		2, 3, 0   // Second triangle
	};

	void createInstance(ImVector<const char*> extensions);
	void createSurface(GLFWwindow* window);
	VkPhysicalDevice pickPhysicalDevice();
	void selectGraphicsQueueFamily();
	void createLogicalDevice();
	void createDescriptorPool();
	void createFrameBuffers(GLFWwindow* window);
	void setupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height);
	void createImguiContext(GLFWwindow* window);
	void frameRender(ImDrawData* draw_data);
	void framePresent();

};

static void check_vk_result(VkResult err);
static bool IsExtensionAvailable(const ImVector<VkExtensionProperties>& properties, const char* extension);
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);

#endif