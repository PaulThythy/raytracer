#ifndef SHADER_H
#define SHADER_H

#include <string>

#include "imgui_impl_vulkan.h"
class Shader {
private:
    VkDevice m_device;

public:
    Shader(VkDevice device): m_device(device) {} 
    ~Shader();

    void createGraphicsPipeline();
    VkShaderModule createShaderModule(const std::vector<char>& code);
};

#endif