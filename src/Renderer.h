#ifndef RENDERER_H
#define RENDERER_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN

#include "vulkan/Shader.h"
#include "vulkan/GraphicsPipeline.h"

#include <GLFW/glfw3.h>

class Renderer {
private:
	VkAllocationCallbacks*		g_Allocator = nullptr;
	VkInstance					g_Instance = VK_NULL_HANDLE;
	VkPhysicalDevice			g_PhysicalDevice = VK_NULL_HANDLE;
	VkDevice					g_Device = VK_NULL_HANDLE;
	uint32_t					g_QueueFamily = (uint32_t)-1;
	VkQueue						g_Queue = VK_NULL_HANDLE;
	VkDebugReportCallbackEXT	g_DebugReport = VK_NULL_HANDLE;
	VkPipelineCache				g_PipelineCache = VK_NULL_HANDLE;
	VkDescriptorPool			g_DescriptorPool = VK_NULL_HANDLE;

	GLFWwindow*					m_window;
	ImGuiIO						m_io;

	ImGui_ImplVulkanH_Window	g_MainWindowData;
	int							g_MinImageCount = 2;
	bool						g_SwapChainRebuild = false;

	Shader* 			m_shader;
	GraphicsPipeline* m_graphicsPipeline;

public:
	void init();
	void mainLoop();
	void cleanup();

	void SetupVulkan(ImVector<const char*> instance_extensions);
	void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height);
	void FramePresent(ImGui_ImplVulkanH_Window* wd);
	void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data);
	bool IsExtensionAvailable(const ImVector<VkExtensionProperties>& properties, const char* extension);
	VkPhysicalDevice SetupVulkan_SelectPhysicalDevice();

	void CleanupVulkanWindow();
	void CleanupVulkan();
};

#endif