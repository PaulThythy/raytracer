#include <stdexcept>

#include "Shader.h"
#include "globals/globals.h"


Shader::Shader() {
    auto vertShaderCode = Config::readFile("shaders/build/vert.spv");
    auto fragShaderCode = Config::readFile("shaders/build/frag.spv");

    m_vertShaderModule = createShaderModule(vertShaderCode);
    m_fragShaderModule = createShaderModule(fragShaderCode);
}

Shader::~Shader() {
    destroyShaderModules();
}

VkShaderModule Shader::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void Shader::destroyShaderModules() {
    vkDestroyShaderModule(m_device, m_fragShaderModule, nullptr);
    vkDestroyShaderModule(m_device, m_vertShaderModule, nullptr);
}