#ifndef VULKAN_CONTEXT_H
#define VULKAN_CONTEXT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <stdexcept>
#include <array>
#include <chrono>
#include <algorithm>
#include <optional>
#include <iostream>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <set>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "globals/globals.h"

class VulkanContext {
public:
	void initVulkan(GLFWwindow* window);
	void mainLoop(GLFWwindow* window);
	void cleanupVulkan();

private:
	VkInstance m_instance;
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;

	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;
	uint32_t m_queueFamily = (uint32_t)-1;

	VkSurfaceKHR m_surface;

	VkSwapchainKHR m_swapchain;
	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainImageViews;
	VkExtent2D m_swapchainExtent;
	VkFormat m_swapchainImageFormat;
	std::vector<VkFramebuffer> m_swapchainFramebuffers;
	std::vector<VkFramebuffer> m_uiFramebuffers;

	VkRenderPass m_renderPass;
	VkRenderPass m_uiRenderPass;

	VkPipeline m_graphicsPipeline;
	VkPipelineLayout m_pipelineLayout;
	VkPipelineCache m_pipelineCache;

	VkCommandPool m_commandPool;
	VkCommandPool m_uiCommandPool;
	std::vector<VkCommandBuffer> m_commandBuffers;
	std::vector<VkCommandBuffer> m_uiCommandBuffers;

	/*VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;

	std::vector<VkBuffer> m_uniformBuffers;
	std::vector<VkDeviceMemory> m_uniformBuffersMemory;
	std::vector<void*> m_uniformBuffersMapped;*/

	VkDescriptorPool m_descriptorPool;
	VkDescriptorPool m_uiDescriptorPool;

	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;
	std::vector<VkFence> m_imagesInFlight;

	std::vector<const char*> m_requiredExtensions;

	ImGui_ImplVulkanH_Window* m_wd;
	VkAllocationCallbacks* m_allocator = nullptr;
	VkDebugReportCallbackEXT m_debugReport = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	
	ImGui_ImplVulkanH_Window m_mainWindowData;

	ImGuiIO* m_io;

	uint32_t m_imageCount = 0;
	int m_minImageCount = 2;
	bool m_swapchainRebuild = false;
	const int m_MAX_FRAMES_IN_FLIGHT = 2;
	uint32_t m_currentFrame = 0;
	bool m_framebufferResized = false;

	const std::vector<const char*> m_validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> m_deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	struct QueueFamilyIndices {
		uint32_t m_graphicsFamily;
		uint32_t m_computeFamily;
		uint32_t m_presentFamily;
	};
	QueueFamilyIndices m_queueIndices;

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR m_capabilities;
		std::vector<VkSurfaceFormatKHR> m_formats;
		std::vector<VkPresentModeKHR> m_presentModes;
	};

	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;

		static VkVertexInputBindingDescription getBindingDescription() {
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
		}
	};

	struct UniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	const std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
	};

	const std::vector<uint16_t> indices = {
		0, 1, 2, 2, 3, 0
	};

#ifdef NDEBUG
	bool m_enableValidationLayers = false;
#else
	bool m_enableValidationLayers = true;
#endif

	void createInstance();
	void createLogicalDevice();
	void createRenderPass();
	void createImageViews();
	void createGraphicsPipeline();
	void createSwapchain();
	void recreateSwapchain(GLFWwindow* window);
	void createFramebuffers();
	void createDescriptorPool();
	void createSurface(GLFWwindow* window);
	VkPhysicalDevice pickPhysicalDevice();
	VkShaderModule createShaderModule(const std::vector<char>& shaderCode);
	void createSyncObjects();
	void createUICommandBuffers();
	void recordUICommands(uint32_t bufferIdx);
	void createUICommandPool(VkCommandPool* cmdPool, VkCommandPoolCreateFlags flags);
	void createUIDescriptorPool();
	void createUIFramebuffers();
	void createUIRenderPass();
	void drawFrame(GLFWwindow* window);
	void drawUI();
	void getDeviceQueueIndices();
	VkExtent2D pickSwapchainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);
	VkCommandBuffer beginSingleTimeCommands(VkCommandPool cmdPool);
	void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool cmdPool);
	bool checkDeviceExtensions(VkPhysicalDevice device);
	bool checkValidationLayerSupport();
	void cleanupSwapchain();
	void cleanupUIResources();
	void createCommandBuffers();
	void createCommandPool();
	std::vector<const char*> getRequiredExtensions() const;
	VkPresentModeKHR pickSwapchainPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
	VkSurfaceFormatKHR pickSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	SwapChainSupportDetails querySwapchainSupport(const VkPhysicalDevice& device);
	void setupDebugMessenger();

	void createImguiContext(GLFWwindow* window);
	bool isDeviceSuitable(VkPhysicalDevice device);
};

inline static void check_vk_result(VkResult err) {
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

inline static bool IsExtensionAvailable(const ImVector<VkExtensionProperties>& properties, const char* extension) {
	for (const VkExtensionProperties& p : properties)
		if (strcmp(p.extensionName, extension) == 0)
			return true;
	return false;
}

inline static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	(void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
	fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
	return VK_FALSE;
}

inline static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

inline static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

inline static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

inline static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

#endif