#ifndef VKRENDERER_H
#define VKRENDERER_H

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
#include "application/Camera.h"
#include "math/Vertex.h"
#include "math/Triangle.h"
#include "math/Material.h"
#include "math/Sphere.h"

class VkRenderer {
public:
	void initVulkan(GLFWwindow* window);
	void mainLoop(GLFWwindow* window);
	void cleanupVulkan();

	VkRenderer();

	bool m_framebufferResized = false;

	inline Camera& getRendererCamera() { return m_camera; }

private:
	VkInstance m_instance;
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;

	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

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

	VkCommandPool m_commandPool;
	VkCommandPool m_uiCommandPool;
	std::vector<VkCommandBuffer> m_commandBuffers;
	std::vector<VkCommandBuffer> m_uiCommandBuffers;

	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;

	std::vector<VkBuffer> m_uniformBuffers;
	std::vector<VkDeviceMemory> m_uniformBuffersMemory;
	std::vector<void*> m_uniformBuffersMapped;

	std::vector<Triangle> m_triangles;
	VkBuffer m_triangleBuffer;
	VkDeviceMemory m_triangleBufferMemory;

	std::vector<Sphere> m_spheres;
	VkBuffer m_sphereBuffer;
	VkDeviceMemory m_sphereBufferMemory;

	VkDescriptorPool m_descriptorPool;
	VkDescriptorPool m_uiDescriptorPool;

	VkDescriptorSetLayout m_descriptorSetLayout;
	//TODO add m_uiDescriptorSetLayout
	std::vector<VkDescriptorSet> m_descriptorSets;
	//TODO add m_uiDescriptorSets

	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;
	std::vector<VkFence> m_imagesInFlight;

	std::vector<const char*> m_requiredExtensions;

	VkAllocationCallbacks* m_allocator = nullptr;
	VkDebugReportCallbackEXT m_debugReport = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	
	ImGui_ImplVulkanH_Window m_mainWindowData;

	ImGuiIO* m_io;

	uint32_t m_imageCount = 0;
	const int m_MAX_FRAMES_IN_FLIGHT = 2;
	uint32_t m_currentFrame = 0;

	bool m_show_demo_window = true;
	bool m_show_another_window = false;
	ImVec4 m_clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	Camera m_camera;

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

	const std::vector<Vertex2D> m_vertices = {
		{{-1.0f, -1.0f, 0.0f},  {0.0f, 0.0f}},
		{{-1.0f,  1.0f, 0.0f},  {0.0f, 1.0f}},
		{{ 1.0f,  1.0f, 0.0f},  {1.0f, 1.0f}},
		{{ 1.0f, -1.0f, 0.0f},  {1.0f, 0.0f}}
	};

	const std::vector<uint32_t> m_indices = {0, 1, 2, 2, 3, 0};

	float m_deltaTime = 0.0f;
	float m_lastFrameTime = 0.0f;
	float m_totalTime = 0.0f;

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
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void createData();
	void createUICommandPool();
	void createUIDescriptorPool();
	void createUIFramebuffers();
	void createUIRenderPass();
	void recordUICommands(uint32_t imageIndex);
	void drawFrame(GLFWwindow* window);
	void drawUI();
	void createDescriptorSetLayout();
	void createDescriptorSets();
	void updateUniformBuffer(uint32_t currentImage, float totalTime);
	void createUniformBuffers();
	void createVertexBuffer(const std::vector<Vertex2D>& verticies);
	void createIndexBuffer(const std::vector<uint32_t>& indices);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
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
