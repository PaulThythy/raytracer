#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>

#include "imgui_impl_vulkan.h"

class Shader {

private:
    VkDevice m_device;
    VkShaderModule m_fragShaderModule;
    VkShaderModule m_vertShaderModule;

    VkShaderModule createShaderModule(const std::vector<char>& code);
    void destroyShaderModules();
public:
    Shader();
    ~Shader();
};

#endif