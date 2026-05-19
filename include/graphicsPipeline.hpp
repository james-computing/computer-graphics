#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <iostream>

#include "../include/shader.hpp"
#include "../include/vertex.hpp"

class GraphicsPipeline {
public:
    vk::raii::PipelineLayout pipelineLayout {nullptr};
    vk::raii::Pipeline vkraii {nullptr};

    void create(
        vk::raii::Device const & device,
        vk::Extent2D const & swapChainExtent,
        vk::SampleCountFlagBits msaaSamples,
        vk::raii::DescriptorSetLayout const & descriptorSetLayout,
        vk::Format const * const colorAttachmentFormats,
        vk::Format depthFormat
    );
};