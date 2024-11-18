#include "VkRenderer.h"

VkRenderer::VkRenderer() : m_camera
(
    glm::vec3(6.0f, 6.0f, 6.0f), 
    glm::vec3(0.0f, 0.0f, 0.0f), 
    glm::vec3(0.0f, 0.0f, -1.0f), 
    45.0f, 
    static_cast<float>(Config::INIT_WINDOW_WIDTH)/static_cast<float>(Config::INIT_WINDOW_HEIGHT), 
    0.1f, 
    100.0f
) {}


void VkRenderer::initVulkan(GLFWwindow* window) {
    createInstance();
    setupDebugMessenger();
    createSurface(window);
    m_physicalDevice = pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createCommandPool();
    createFramebuffers();
    createData();
    createVertexBuffer(m_vertices);
    createIndexBuffer(m_indices);
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();

    createImguiContext(window);
}

void VkRenderer::cleanupVulkan() {
    VkResult err = vkDeviceWaitIdle(m_device);
    check_vk_result(err);

    //TODO delete m_io ?

    //cleaning imgui context
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    //cleaning vulkan context
    ImGui_ImplVulkanH_DestroyWindow(m_instance, m_device, &m_mainWindowData, m_allocator);

    vkFreeCommandBuffers(m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
    vkFreeCommandBuffers(m_device, m_uiCommandPool, static_cast<uint32_t>(m_uiCommandBuffers.size()), m_uiCommandBuffers.data());

    //TODO free descriptor sets only if the flag VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT is enabled

    vkDestroyCommandPool(m_device, m_commandPool, m_allocator);
    vkDestroyCommandPool(m_device, m_uiCommandPool, m_allocator);

    vkDestroyPipeline(m_device, m_graphicsPipeline, m_allocator);
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, m_allocator);
    vkDestroyRenderPass(m_device, m_renderPass, m_allocator);
    vkDestroyRenderPass(m_device, m_uiRenderPass, m_allocator);

    vkDestroyDescriptorPool(m_device, m_descriptorPool, m_allocator);
    vkDestroyDescriptorPool(m_device, m_uiDescriptorPool, m_allocator);

    vkDestroyBuffer(m_device, m_vertexBuffer, m_allocator);
    vkFreeMemory(m_device, m_vertexBufferMemory, m_allocator);

    vkDestroyBuffer(m_device, m_indexBuffer, m_allocator);
    vkFreeMemory(m_device, m_indexBufferMemory, m_allocator);

    vkDestroyBuffer(m_device, m_triangleBuffer, m_allocator);
    vkFreeMemory(m_device, m_triangleBufferMemory, m_allocator);

    vkDestroyBuffer(m_device, m_sphereBuffer, m_allocator);
    vkFreeMemory(m_device, m_sphereBufferMemory, m_allocator);

    for (size_t i = 0; i < m_uniformBuffers.size(); i++) {
        vkDestroyBuffer(m_device, m_uniformBuffers[i], m_allocator);
        vkFreeMemory(m_device, m_uniformBuffersMemory[i], m_allocator);
    }

    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, m_allocator);

    for (size_t i = 0; i < m_MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], m_allocator);
        vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], m_allocator);
        vkDestroyFence(m_device, m_inFlightFences[i], m_allocator);
    }

    //if using the debug report callback
    auto f_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT");
    if (f_vkDestroyDebugReportCallbackEXT != nullptr) {
        f_vkDestroyDebugReportCallbackEXT(m_instance, m_debugReport, m_allocator);
    }

    if (m_enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, m_allocator);
    }

    for (auto& swapchainFramebuffer : m_swapchainFramebuffers) {
        vkDestroyFramebuffer(m_device, swapchainFramebuffer, m_allocator);
    }

    for (auto& uiFramebuffer : m_uiFramebuffers) {
        vkDestroyFramebuffer(m_device, uiFramebuffer, m_allocator);
    }

    for (auto& swapchainImageView : m_swapchainImageViews) {
        vkDestroyImageView(m_device, swapchainImageView, m_allocator);
    }
    vkDestroySwapchainKHR(m_device, m_swapchain, m_allocator);
    vkDestroySurfaceKHR(m_instance, m_surface, m_allocator);
    vkDestroyDevice(m_device, m_allocator);
    vkDestroyInstance(m_instance, m_allocator);
}

bool VkRenderer::checkDeviceExtensions(VkPhysicalDevice device) {
    uint32_t extensionsCount = 0;
    if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("Unable to enumerate device extensions!");
    }

    std::vector<VkExtensionProperties> availableExtensions(extensionsCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, availableExtensions.data());

    std::set<std::string> requiredDeviceExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());
    for (const auto& extension : availableExtensions) {
        requiredDeviceExtensions.erase(extension.extensionName);
    }

    return requiredDeviceExtensions.empty();
}

bool VkRenderer::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : m_validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }
    return true;
}

void VkRenderer::createData() {
    glm::vec3 planeNormal(0.0f, 0.0f, 1.0f); // Assuming plane is in XY plane

    m_triangles = {
       Triangle(Vertex3D({-10.0f, 10.0f, 0.0f}, planeNormal),
                 Vertex3D({10.0f, 10.0f, 0.0f}, planeNormal),
                 Vertex3D({10.0f, -10.0f, 0.0f}, planeNormal),
                 Material({0.4, 0.4, 0.4}, {0.0f, 0.0f, 0.0f}, 0.0f, 0.2f, 0.0f)
        ),
        Triangle(Vertex3D({10.0f, -10.0f, 0.0f}, planeNormal),
                 Vertex3D({-10.0f, -10.0f, 0.0f}, planeNormal),
                 Vertex3D({-10.0f, 10.0f, 0.0f}, planeNormal),
                 Material({0.4, 0.4, 0.4}, {0.0f, 0.0f, 0.0f}, 0.0f, 0.2f, 0.0f)
        )
    };

    Material mat({1.0f, 0.9f, 0.0f}, {0.0f, 0.0f, 0.0f}, 0.0f, 0.2f, 1.0f);
    Sphere sphere({0.0, 0.0, 1.0}, 1.0, mat);
    /*std::vector<Triangle> sphereGeom = sphere.sphereGeometry(5, 5);
    m_triangles.insert(std::end(m_triangles), std::begin(sphereGeom), std::end(sphereGeom));*/
    m_spheres = {
        sphere
    };

    VkDeviceSize triangleBufferSize;

    if (m_triangles.empty()) {
        triangleBufferSize = sizeof(Triangle);
        createBuffer(triangleBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_triangleBuffer, m_triangleBufferMemory);

        void* data;
        vkMapMemory(m_device, m_triangleBufferMemory, 0, triangleBufferSize, 0, &data);
        memset(data, 0, static_cast<size_t>(triangleBufferSize));
        vkUnmapMemory(m_device, m_triangleBufferMemory);
    }
    else {
        triangleBufferSize = sizeof(Triangle) * m_triangles.size();

        createBuffer(triangleBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_triangleBuffer, m_triangleBufferMemory);

        void* data;
        vkMapMemory(m_device, m_triangleBufferMemory, 0, triangleBufferSize, 0, &data);
        memcpy(data, m_triangles.data(), static_cast<size_t>(triangleBufferSize));
        vkUnmapMemory(m_device, m_triangleBufferMemory);
    }

    VkDeviceSize sphereBufferSize;

    if (m_spheres.empty()) {
        sphereBufferSize = sizeof(Sphere);
        createBuffer(sphereBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_sphereBuffer, m_sphereBufferMemory);

        void* data;
        vkMapMemory(m_device, m_sphereBufferMemory, 0, sphereBufferSize, 0, &data);
        memset(data, 0, static_cast<size_t>(sphereBufferSize));
        vkUnmapMemory(m_device, m_sphereBufferMemory);
    }
    else {
        sphereBufferSize = sizeof(Sphere) * m_spheres.size();

        createBuffer(sphereBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_sphereBuffer, m_sphereBufferMemory);

        void* data;
        vkMapMemory(m_device, m_sphereBufferMemory, 0, sphereBufferSize, 0, &data);
        memcpy(data, m_spheres.data(), static_cast<size_t>(sphereBufferSize));
        vkUnmapMemory(m_device, m_sphereBufferMemory);
    }
}

void VkRenderer::createDescriptorSetLayout() {
    //camera
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    //SSBO for triangles
    VkDescriptorSetLayoutBinding triangleBufferLayoutBinding{};
    triangleBufferLayoutBinding.binding = 1;
    triangleBufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    triangleBufferLayoutBinding.descriptorCount = 1;
    triangleBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    triangleBufferLayoutBinding.pImmutableSamplers = nullptr;

    //SSBO for spheres
    VkDescriptorSetLayoutBinding sphereBufferLayoutBinding{};
    sphereBufferLayoutBinding.binding = 2;
    sphereBufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    sphereBufferLayoutBinding.descriptorCount = 1;
    sphereBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    sphereBufferLayoutBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 3> bindings = {uboLayoutBinding, triangleBufferLayoutBinding, sphereBufferLayoutBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
}

void VkRenderer::createDescriptorSets() {
    m_descriptorSets.resize(m_swapchainImages.size());

    // Create a vector of descriptor layouts, one for each swapchain image
    std::vector<VkDescriptorSetLayout> layouts(m_swapchainImages.size(), m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool; 
    allocInfo.descriptorSetCount = static_cast<uint32_t>(m_swapchainImages.size()); 
    allocInfo.pSetLayouts = layouts.data();  

    // Allocation des Descriptor Sets
    if (vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate of descriptor sets !");
    }

    // For each Descriptor Set, link the corresponding uniform buffer
    for (size_t i = 0; i < m_swapchainImages.size(); i++) {
        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniformBuffers[i]; 
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(Camera::UniformBufferObject);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_descriptorSets[i];  
        descriptorWrites[0].dstBinding = 0;  
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;  
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;  

        VkDescriptorBufferInfo triangleBufferInfo{};
        triangleBufferInfo.buffer = m_triangleBuffer;
        triangleBufferInfo.offset = 0;
        triangleBufferInfo.range = m_triangles.empty() ? sizeof(Triangle) : sizeof(Triangle) * m_triangles.size();

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &triangleBufferInfo;

        VkDescriptorBufferInfo sphereBufferInfo{};
        sphereBufferInfo.buffer = m_sphereBuffer;
        sphereBufferInfo.offset = 0;
        sphereBufferInfo.range = m_spheres.empty() ? sizeof(Sphere) : sizeof(Sphere) * m_spheres.size();

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = m_descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &sphereBufferInfo;

        vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void VkRenderer::createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 3> poolSizes{};

    //for camera
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(m_swapchainImages.size());

    //for triangles
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(m_swapchainImages.size());

    //for spheres
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(m_swapchainImages.size());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(m_swapchainImages.size());
    //poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool !");
    }
}

void VkRenderer::createCommandPool() {
    VkCommandPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.queueFamilyIndex = m_queueIndices.m_graphicsFamily;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(m_device, &createInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create command pool!");
    }
}

void VkRenderer::createUICommandPool() {
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = m_queueIndices.m_graphicsFamily;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_uiCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("Could not create graphics command pool!");
    }
}

void VkRenderer::createFramebuffers() {
    m_swapchainFramebuffers.resize(m_swapchainImageViews.size());
    for (size_t i = 0; i < m_swapchainImageViews.size(); ++i) {
        // We need to attach an image view to the frame buffer for presentation purposes
        VkImageView attachments[] = { m_swapchainImageViews[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapchainExtent.width;
        framebufferInfo.height = m_swapchainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapchainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create framebuffer!");
        }
    }
}

void VkRenderer::createGraphicsPipeline() {
    // Load our shader modules in from disk
    auto vertShaderCode = Config::readFile(std::string(SHADER_DIR) + "/build/vert.spv");
    auto fragShaderCode = Config::readFile(std::string(SHADER_DIR) + "/build/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Vertex2D::getBindingDescription();
    auto attributeDescriptions = Vertex2D::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;

    if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(m_device, fragShaderModule, m_allocator);
    vkDestroyShaderModule(m_device, vertShaderModule, m_allocator);
}

void VkRenderer::createImageViews() {
    m_swapchainImageViews.resize(m_swapchainImages.size());
    for (size_t i = 0; i < m_swapchainImages.size(); ++i) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_swapchainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // We do not enable mip-mapping
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_device, &createInfo, nullptr, &m_swapchainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create image views!");
        }
    }
}

void VkRenderer::createInstance() {
    if (m_enableValidationLayers && !checkValidationLayerSupport()) {
        //throw std::runtime_error("Unable to establish validation layer support!");
        std::cerr << "Unable to establish validation layer support!" << std::endl;
        m_enableValidationLayers = false; 
    }

    VkApplicationInfo applicationInfo;
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "Vulkan Raytracer";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "No Engine";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_2;
    applicationInfo.pNext = nullptr;

    // Grab the needed Vulkan extensions. This also initializes the list of required extensions
    m_requiredExtensions = getRequiredExtensions();
    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_requiredExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = m_requiredExtensions.data();

    // Enable validation layers and debug messenger if needed
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (m_enableValidationLayers) {
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = m_validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        debugCreateInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
    }
    else {
        instanceCreateInfo.enabledLayerCount = 0;
        debugCreateInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create a Vulkan instance!");
    }
}

void VkRenderer::createLogicalDevice() {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
    if (queueFamilyCount == 0) {
        throw std::runtime_error("Failed to retrieve queue families.");
    }

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

    int graphicsFamilyIndex = -1;
    int presentFamilyIndex = -1;

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        // V�rifier le support des op�rations graphiques
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamilyIndex = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &presentSupport);
        if (presentSupport) {
            presentFamilyIndex = i;
        }

        if (graphicsFamilyIndex != -1 && presentFamilyIndex != -1) {
            break;
        }
    }

    if (graphicsFamilyIndex == -1 || presentFamilyIndex == -1) {
        throw std::runtime_error("Unable to find the required queue families !");
    }

    m_queueIndices.m_graphicsFamily = graphicsFamilyIndex;
    m_queueIndices.m_presentFamily = presentFamilyIndex;

    std::set<uint32_t> uniqueQueueIndices = { m_queueIndices.m_graphicsFamily, m_queueIndices.m_presentFamily };
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    const float priority = 1.0f;
    for (uint32_t queueIndex : uniqueQueueIndices) {
        VkDeviceQueueCreateInfo queueInfo = {};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = queueIndex;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &priority;
        queueCreateInfos.push_back(queueInfo);
    }

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
    deviceInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

    VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
    physicalDeviceFeatures.fragmentStoresAndAtomics = VK_TRUE;
    deviceInfo.pEnabledFeatures = &physicalDeviceFeatures;

    if (m_enableValidationLayers) {
        deviceInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
        deviceInfo.ppEnabledLayerNames = m_validationLayers.data();
    }
    else {
        deviceInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_physicalDevice, &deviceInfo, nullptr, &m_device) != VK_SUCCESS) {
        throw std::runtime_error("Impossible de cr�er le dispositif logique !");
    }

    vkGetDeviceQueue(m_device, m_queueIndices.m_graphicsFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_queueIndices.m_presentFamily, 0, &m_presentQueue);
}

void VkRenderer::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Could not create render pass!");
    }
}

VkShaderModule VkRenderer::createShaderModule(const std::vector<char>& shaderCode) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create shader module!");
    }

    return shaderModule;
}

void VkRenderer::updateUniformBuffer(uint32_t currentImage, float deltaTime) {
    Camera::UniformBufferObject ubo;
    //ubo.m_model = glm::rotate(glm::mat4(1.0f), deltaTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    m_camera.m_cameraUBO.m_aspectRatio = static_cast<float>(m_swapchainExtent.width)/static_cast<float>(m_swapchainExtent.height);
    m_camera.updateCameraUBO(ubo, deltaTime);

    memcpy(m_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void VkRenderer::createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(Camera::UniformBufferObject);

    size_t imageCount = m_swapchainImages.size();

    m_uniformBuffers.resize(imageCount);
    m_uniformBuffersMemory.resize(imageCount);
    m_uniformBuffersMapped.resize(imageCount);

    for (size_t i = 0; i < imageCount; i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]);

        vkMapMemory(m_device, m_uniformBuffersMemory[i], 0, bufferSize, 0, &m_uniformBuffersMapped[i]);
    }
}

void VkRenderer::createVertexBuffer(const std::vector<Vertex2D>& vertices) {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    // Create a temporary memory buffer accessible by the CPU
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    // Copy vertex data to the temporary buffer
    void* data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    // Create vertex buffer in GPU memory
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

    // Copy data from the temporary buffer to the vertex buffer on the GPU
    copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

    // Clean temporary buffer
    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void VkRenderer::createIndexBuffer(const std::vector<uint32_t>& indices) {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

    copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void VkRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
}

void VkRenderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(m_commandPool);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer, m_commandPool);
}

uint32_t VkRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

// For cross-platform compatibility we let GLFW take care of the surface creation
void VkRenderer::createSurface(GLFWwindow* window) {
    if (glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create window surface!");
    }
}

void VkRenderer::createSwapchain() {
    SwapChainSupportDetails configuration = querySwapchainSupport(m_physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = pickSwapchainSurfaceFormat(configuration.m_formats);
    VkExtent2D extent = pickSwapchainExtent(configuration.m_capabilities);
    VkPresentModeKHR presentMode = pickSwapchainPresentMode(configuration.m_presentModes);

    m_imageCount = configuration.m_capabilities.minImageCount + 1;
    if (configuration.m_capabilities.maxImageCount > 0 && m_imageCount > configuration.m_capabilities.maxImageCount) {
        // In case we are exceeding the maximum capacity for swap chain images we reset the value
        m_imageCount = configuration.m_capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = m_surface;
    swapchainCreateInfo.minImageCount = m_imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // TODO Change this later to setup for compute

    // Check if the graphics and present queues are the same and setup
    // sharing of the swap chain accordingly
    uint32_t indices[] = { m_queueIndices.m_graphicsFamily, m_queueIndices.m_presentFamily };
    if (m_queueIndices.m_presentFamily == m_queueIndices.m_graphicsFamily) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = indices;
    }

    swapchainCreateInfo.preTransform = configuration.m_capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create swap chain!");
    }

    m_swapchainImageFormat = surfaceFormat.format;
    m_swapchainExtent = extent;

    // Store the handles to the swap chain images for later use
    uint32_t swapchainCount;
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchainCount, nullptr);
    m_swapchainImages.resize(swapchainCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchainCount, m_swapchainImages.data());
}

void VkRenderer::createSyncObjects() {
    // Create our semaphores and fences for synchronizing the GPU and CPU
    m_imageAvailableSemaphores.resize(m_MAX_FRAMES_IN_FLIGHT);
    m_renderFinishedSemaphores.resize(m_MAX_FRAMES_IN_FLIGHT);
    m_inFlightFences.resize(m_MAX_FRAMES_IN_FLIGHT);
    m_imagesInFlight.resize(m_swapchainImages.size(), VK_NULL_HANDLE);

    // Create semaphores for the number of frames that can be submitted to the GPU at a time
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Set this flag to be initialized to be on
    for (size_t i = 0; i < m_MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create semaphore!");
        }

        if (vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create semaphore!");
        }

        if (vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create fence!");
        }
    }
}

void VkRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    // Begin render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_swapchainExtent;

    VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Bind the graphics pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

    // Set the dynamic viewport
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapchainExtent.width);
    viewport.height = static_cast<float>(m_swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    // Set the dynamic scissor
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_swapchainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // Bind vertex and index buffers
    VkBuffer vertexBuffers[] = { m_vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    // Bind descriptor sets (if any)
    vkCmdBindDescriptorSets(
        commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout,
        0, 1, &m_descriptorSets[imageIndex], 0, nullptr
    );

    // Issue draw command
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);

    // End the render pass
    vkCmdEndRenderPass(commandBuffer);

    // Transition the swapchain image layout to PRESENT_SRC_KHR
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_swapchainImages[imageIndex];
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    // End command buffer recording
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!");
    }
}

void VkRenderer::createUIDescriptorPool() {
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(pool_sizes));
    pool_info.pPoolSizes = pool_sizes;
    if (vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_uiDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Cannot allocate UI descriptor pool!");
    }
}

void VkRenderer::createUIFramebuffers() {
    // Create some UI framebuffers. These will be used in the render pass for the UI
    m_uiFramebuffers.resize(m_swapchainImages.size());
    VkImageView attachment[1];
    VkFramebufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = m_uiRenderPass;
    info.attachmentCount = 1;
    info.pAttachments = attachment;
    info.width = m_swapchainExtent.width;
    info.height = m_swapchainExtent.height;
    info.layers = 1;
    for (uint32_t i = 0; i < m_swapchainImages.size(); ++i) {
        attachment[0] = m_swapchainImageViews[i];
        if (vkCreateFramebuffer(m_device, &info, nullptr, &m_uiFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create UI framebuffers!");
        }
    }
}

void VkRenderer::createUIRenderPass() {
    // Create an attachment description for the render pass
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = m_swapchainImageFormat;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // Need UI to be drawn on top of main
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Last pass so we want to present after
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // Create a color attachment reference
    VkAttachmentReference attachmentReference = {};
    attachmentReference.attachment = 0;
    attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Create a subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachmentReference;

    // Create a subpass dependency to synchronize our main and UI render passes
    // We want to render the UI after the geometry has been written to the framebuffer
    // so we need to configure a subpass dependency as such
    VkSubpassDependency subpassDependency = {};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Create external dependency
    subpassDependency.dstSubpass = 0; // The geometry subpass comes first
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Wait on writes
    subpassDependency.dstStageMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Finally create the UI render pass
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &attachmentDescription;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &subpassDependency;

    if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_uiRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create UI render pass!");
    }
}

void VkRenderer::recordUICommands(uint32_t imageIndex) {
    VkCommandBufferBeginInfo cmdBufferBegin = {};
    cmdBufferBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferBegin.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(m_uiCommandBuffers[imageIndex], &cmdBufferBegin) != VK_SUCCESS) {
        throw std::runtime_error("Unable to start recording UI command buffer!");
    }

    VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = m_uiRenderPass;
    renderPassBeginInfo.framebuffer = m_uiFramebuffers[imageIndex];
    renderPassBeginInfo.renderArea.extent.width = m_swapchainExtent.width;
    renderPassBeginInfo.renderArea.extent.height = m_swapchainExtent.height;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(m_uiCommandBuffers[imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Grab and record the draw data for Dear Imgui
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_uiCommandBuffers[imageIndex]);

    // End and submit render pass
    vkCmdEndRenderPass(m_uiCommandBuffers[imageIndex]);

    if (vkEndCommandBuffer(m_uiCommandBuffers[imageIndex]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffers!");
    }
}

void VkRenderer::drawFrame(GLFWwindow* window) {
    // Calculate deltaTime
    float currentFrameTime = static_cast<float>(glfwGetTime());
    m_deltaTime = currentFrameTime - m_lastFrameTime;
    m_totalTime += m_deltaTime;
    m_lastFrameTime = currentFrameTime;

    // Sync for next frame. Fences also need to be manually reset unlike semaphores, which is done here
    vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

    // If the window has been resized or another event causes the swap chain to become invalid,
    // we need to recreate it
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain(window);
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Unable to acquire swap chain!");
    }

    updateUniformBuffer(imageIndex, m_deltaTime);

    // Check if a previous frame is using this image (i.e. there is its fence to wait on)
    if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(m_device, 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }

    // Mark the image as now being in use by this frame
    m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

    // Ensure the primary command buffer is recorded for the current image
    vkResetCommandBuffer(m_commandBuffers[imageIndex], 0);
    recordCommandBuffer(m_commandBuffers[imageIndex], imageIndex);

    // Record UI command buffer if necessary
    recordUICommands(imageIndex);


    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    std::array<VkCommandBuffer, 2> cmdBuffers = { m_commandBuffers[imageIndex], m_uiCommandBuffers[imageIndex] };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = static_cast<uint32_t>(cmdBuffers.size());
    submitInfo.pCommandBuffers = cmdBuffers.data();

    VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // Reset the in-flight fences so we do not get blocked waiting on in-flight images
    vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

    if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = { m_swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    // Submit the present queue
    result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized) {
        // Recreate the swap chain if the window size has changed
        m_framebufferResized = false;
        recreateSwapchain(window);
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("Unable to present the swap chain image!");
    }

    // Advance the current frame to get the semaphore data for the next frame
    m_currentFrame = (m_currentFrame + 1) % m_MAX_FRAMES_IN_FLIGHT;
}

void VkRenderer::drawUI() {
    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (m_show_demo_window)
        ImGui::ShowDemoWindow(&m_show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &m_show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &m_show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&m_clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_io->Framerate, m_io->Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (m_show_another_window)
    {
        ImGui::Begin("Another Window", &m_show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            m_show_another_window = false;
        ImGui::End();
    }

    ImGui::Render();
}

VkCommandBuffer VkRenderer::beginSingleTimeCommands(VkCommandPool cmdPool) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = cmdPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = {};
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Could not create one-time command buffer!");
    }

    return commandBuffer;
}

void VkRenderer::endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool cmdPool) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(m_device, cmdPool, 1, &commandBuffer);
}

void VkRenderer::getDeviceQueueIndices() {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueProperties.data());
    for (size_t i = 0; i < queueProperties.size(); ++i) {
        if (queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_queueIndices.m_graphicsFamily = i;
        }

        // Check if a queue supports presentation
        // If this returns false, make sure to enable DRI3 if using X11
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &presentSupport);
        if (presentSupport) {
            m_queueIndices.m_presentFamily = i;
        }

        if (queueProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            m_queueIndices.m_computeFamily = i;
        }
    }
}

void VkRenderer::createCommandBuffers() {
    m_commandBuffers.resize(m_swapchainImages.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void VkRenderer::createUICommandBuffers() {
    m_uiCommandBuffers.resize(m_swapchainImages.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_uiCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_uiCommandBuffers.size());

    if (vkAllocateCommandBuffers(m_device, &allocInfo, m_uiCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Unable to allocate UI command buffers!");
    }
}

std::vector<const char*> VkRenderer::getRequiredExtensions() const {
    uint32_t glfwExtensionCount = 0;
    const char** glfwRequiredExtensions;
    glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwRequiredExtensions, glfwRequiredExtensions + glfwExtensionCount);
    for (const char* extension : extensions) {
        std::cout << extension << std::endl;
    }

    if (m_enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
}

void VkRenderer::createImguiContext(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    m_io = &ImGui::GetIO(); (void)m_io;
    m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
#ifdef _WIN32
    //m_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    //m_io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
#endif

    // Initialize some DearImgui specific resources
    createUIDescriptorPool();
    createUIRenderPass();
    createUICommandPool();
    createUICommandBuffers();
    createUIFramebuffers();

    // Provide bind points from Vulkan API
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_instance;
    init_info.PhysicalDevice = m_physicalDevice;
    init_info.Device = m_device;
    init_info.QueueFamily = m_queueIndices.m_graphicsFamily;
    init_info.Queue = m_graphicsQueue;
    init_info.DescriptorPool = m_uiDescriptorPool;
    init_info.MinImageCount = m_imageCount;
    init_info.ImageCount = m_imageCount;
    init_info.RenderPass = m_uiRenderPass;
    ImGui_ImplVulkan_Init(&init_info);
}

bool VkRenderer::isDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    bool extensionsSupported = checkDeviceExtensions(device);

    // Check if Swap Chain support is adequate
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapchainConfig = querySwapchainSupport(device);
        swapChainAdequate = !swapchainConfig.m_presentModes.empty() && !swapchainConfig.m_formats.empty();
    }

    return swapChainAdequate && extensionsSupported && properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

VkPhysicalDevice VkRenderer::pickPhysicalDevice() {
    uint32_t physicalDeviceCount = 0;
    if (vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("Unable to enumerate physical devices!");
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data());

    if (physicalDevices.empty()) {
        throw std::runtime_error("No physical devices are available that support Vulkan!");
    }

    for (const auto& device : physicalDevices) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        if (isDeviceSuitable(device)) {
            std::cout << "Using discrete GPU: " << properties.deviceName << std::endl;
            return device;
        }
    }

    // Did not find a discrete GPU, pick the first device from the list as a fallback
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevices[0], &properties);
    // Need to check if this is at least a physical device with GPU capabilities
    if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        throw std::runtime_error("Did not find a physical GPU on this system!");
    }

    std::cout << "Using fallback physical device: " << properties.deviceName << std::endl;
    return physicalDevices[0];
}

VkExtent2D VkRenderer::pickSwapchainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities) {
    if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
        return surfaceCapabilities.currentExtent;
    }
    else {
        VkExtent2D actualExtent = { Config::INIT_WINDOW_WIDTH, Config::INIT_WINDOW_HEIGHT };
        actualExtent.width = std::max(surfaceCapabilities.minImageExtent.width,
            std::min(surfaceCapabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(surfaceCapabilities.minImageExtent.height,
            std::min(surfaceCapabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }
}

VkPresentModeKHR VkRenderer::pickSwapchainPresentMode(const std::vector<VkPresentModeKHR>& presentModes) {
    // Look for triple-buffering present mode if available
    for (VkPresentModeKHR availableMode : presentModes) {
        if (availableMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availableMode;
        }
    }

    // Use FIFO mode as our fallback, since it's the only guaranteed mode
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR VkRenderer::pickSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
    for (VkSurfaceFormatKHR availableFormat : formats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    // As a fallback choose the first format
    // TODO Could establish a ranking and pick best one
    return formats[0];
}

VkRenderer::SwapChainSupportDetails VkRenderer::querySwapchainSupport(const VkPhysicalDevice& device) {
    SwapChainSupportDetails config = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &config.m_capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
    if (formatCount > 0) {
        config.m_formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, config.m_formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);
    if (presentModeCount > 0) {
        config.m_presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, config.m_presentModes.data());
    }

    return config;
}

// Destroys all the resources associated with swapchain recreation
void VkRenderer::cleanupSwapchain() {
    for (auto& swapchainFramebuffer : m_swapchainFramebuffers) {
        vkDestroyFramebuffer(m_device, swapchainFramebuffer, m_allocator);
    }

    vkFreeCommandBuffers(m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());

    vkDestroyPipeline(m_device, m_graphicsPipeline, m_allocator);
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, m_allocator);
    vkDestroyRenderPass(m_device, m_renderPass, m_allocator);

    for (auto& swapchainImageView : m_swapchainImageViews) {
        vkDestroyImageView(m_device, swapchainImageView, m_allocator);
    }

    vkDestroySwapchainKHR(m_device, m_swapchain, m_allocator);
}

void VkRenderer::cleanupUIResources() {
    for (auto framebuffer : m_uiFramebuffers) {
        vkDestroyFramebuffer(m_device, framebuffer, m_allocator);
    }

    vkFreeCommandBuffers(m_device, m_uiCommandPool, static_cast<uint32_t>(m_uiCommandBuffers.size()), m_uiCommandBuffers.data());
}

// In case the swapchain is invalidated, i.e. during window resizing,
// we need to implement a mechanism to recreate it
void VkRenderer::recreateSwapchain(GLFWwindow* window) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        // Idle wait in case our application has been minimized
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_device);

    cleanupSwapchain();
    cleanupUIResources();

    // Recreate main application Vulkan resources
    createSwapchain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createDescriptorPool();
    createCommandBuffers();

    // We also need to take care of the UI
    ImGui_ImplVulkan_SetMinImageCount(m_imageCount);
    createUICommandBuffers();
    createUIFramebuffers();
}

void VkRenderer::mainLoop(GLFWwindow* window) {
    drawUI();
    drawFrame(window);

    // Wait for unfinished work on GPU before ending application
    vkDeviceWaitIdle(m_device);
}

void VkRenderer::setupDebugMessenger() {
    if (!m_enableValidationLayers) {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Failed to setup debug messenger!");
    }
}