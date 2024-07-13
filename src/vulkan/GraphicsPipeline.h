#ifndef GRAPHICSPIPELINE_H
#define GRAPHICSPIPELINE_H

#include "imgui_impl_vulkan.h"

namespace rtVulkan {

    struct GraphicsPipeline {
        VkDevice m_device;
        VkPipeline m_graphicsPipeline;
        VkPipelineLayout m_pipelineLayout;

        GraphicsPipeline(VkDevice device, VkRenderPass renderPass,
                        const VkDescriptorSetLayout descriptorSetLayout,
                        const VkShaderModule vertShaderModule,
                        const VkShaderModule fragShaderModule);

        ~GraphicsPipeline() {
            vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
            vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
        }

        void createPipeline(    VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout,
                                VkShaderModule vertShaderModule, VkShaderModule fragShaderModule);


    };
}

#endif