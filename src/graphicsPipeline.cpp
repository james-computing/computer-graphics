#include "../include/graphicsPipeline.hpp"

void GraphicsPipeline::create(
    vk::raii::Device const & device,
    vk::Extent2D const & swapChainExtent,
    vk::SampleCountFlagBits msaaSamples,
    vk::Format const * const colorAttachmentFormats,
    vk::Format depthFormat,
    std::vector<vk::DescriptorSetLayout> const & descriptorSetLayouts
) {
    std::vector<char> const shaderCode {Shader::readFile("shaders/slang.spv")};
    std::cout << "Shader code size = " << shaderCode.size() << " bytes" << std::endl;

    // The shader module is only needed during the pipeline creation,
    // so we can keep it as a local variable for this method.
    vk::raii::ShaderModule const shaderModule = Shader::createShaderModule(device, shaderCode);

    vk::PipelineShaderStageCreateInfo const vertShaderStageCreateInfo {
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = shaderModule,
        .pName = "vertMain"
    };

    vk::PipelineShaderStageCreateInfo const fragShaderStageCreateInfo {
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = shaderModule,
        .pName = "fragMain"
    };

    vk::PipelineShaderStageCreateInfo const shaderStageCreateInfos[] {vertShaderStageCreateInfo, fragShaderStageCreateInfo};

    std::vector<vk::DynamicState> const dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo const pipelineDynamicStateCreateInfo {
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };


    vk::VertexInputBindingDescription constexpr bindingDescription {Vertex::getBindingDescription()};
    std::array<vk::VertexInputAttributeDescription, 2> constexpr attributeDescriptions {Vertex::getAttributeDescriptions()};
    vk::PipelineVertexInputStateCreateInfo const vertexInputCreateInfo {
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data()
    };

    vk::PipelineInputAssemblyStateCreateInfo constexpr inputAssemblyCreateInfo {
        .topology = vk::PrimitiveTopology::eTriangleList // triangle from every 3 vertices, without reuse.
    };

    vk::Viewport const viewport {
        .x = 0,
        .y = 0,
        .width = static_cast<float>(swapChainExtent.width),
        .height = static_cast<float>(swapChainExtent.height),
        .minDepth = 0,
        .maxDepth = 1
    };

    vk::Rect2D const rect2D {
        .offset = vk::Offset2D{0,0},
        .extent = swapChainExtent
    };

    vk::PipelineViewportStateCreateInfo constexpr pipelineViewportStateCreateInfo {
        .viewportCount = 1,
        .scissorCount = 1
    };

    vk::PipelineRasterizationStateCreateInfo constexpr pipelineRasterizationStateCreateInfo {
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eCounterClockwise,
        .depthBiasEnable = vk::False,
        .depthBiasSlopeFactor = 1.0f,
        .lineWidth = 1.0f
    };

    vk::PipelineMultisampleStateCreateInfo const pipelineMultisampleStateCreateInfo {
        .rasterizationSamples = msaaSamples,
        .sampleShadingEnable = vk::False
    };

    vk::PipelineColorBlendAttachmentState constexpr pipelineColorBlendAttachmentState {
        .blendEnable =      vk::False,
        .colorWriteMask =   vk::ColorComponentFlagBits::eR |
                            vk::ColorComponentFlagBits::eG |
                            vk::ColorComponentFlagBits::eB |
                            vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo const pipelineColorBlendStateCreateInfo {
        .logicOpEnable =    vk::False,
        .logicOp =          vk::LogicOp::eCopy,
        .attachmentCount =  1,
        .pAttachments =     &pipelineColorBlendAttachmentState
    };

    vk::PipelineLayoutCreateInfo const pipelineLayoutCreateInfo {
        .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
        .pSetLayouts = descriptorSetLayouts.data(),
        .pushConstantRangeCount = 0
    };

    pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutCreateInfo);

    vk::PipelineRenderingCreateInfo const pipelineRenderingCreateInfo {
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = colorAttachmentFormats,
        .depthAttachmentFormat = depthFormat
    };

    vk::PipelineDepthStencilStateCreateInfo constexpr depthStencilStateCreateInfo {
        .depthTestEnable = vk::True,
        .depthWriteEnable = vk::True,
        .depthCompareOp = vk::CompareOp::eLess,
        .depthBoundsTestEnable = vk::False,
        .stencilTestEnable = vk::False
    };

    vk::GraphicsPipelineCreateInfo const graphicsPipelineCreateInfo {
        .pNext =                &pipelineRenderingCreateInfo,
        .stageCount =           2,
        .pStages =              shaderStageCreateInfos,
        .pVertexInputState =    &vertexInputCreateInfo,
        .pInputAssemblyState =  &inputAssemblyCreateInfo,
        .pViewportState =       &pipelineViewportStateCreateInfo,
        .pRasterizationState =  &pipelineRasterizationStateCreateInfo,
        .pMultisampleState =    &pipelineMultisampleStateCreateInfo,
        .pDepthStencilState =   &depthStencilStateCreateInfo,
        .pColorBlendState =     &pipelineColorBlendStateCreateInfo,
        .pDynamicState =        &pipelineDynamicStateCreateInfo,
        .layout =               pipelineLayout,
        .renderPass =           nullptr, // because using dynamic rendering
        .basePipelineHandle =   VK_NULL_HANDLE, // optional
        .basePipelineIndex =    -1 // optional
    };

    // try catch?
    vkraii = vk::raii::Pipeline(device, nullptr, graphicsPipelineCreateInfo);
}