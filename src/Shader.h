#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>

#include "imgui_impl_vulkan.h"
class Shader {
private:
    VkDevice m_device;
    VkShaderModule createShaderModule(const std::vector<char>& code);
public:
    void setDevice(VkDevice device) { m_device = device; }
    void createGraphicsPipeline();
};

#endif