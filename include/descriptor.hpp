#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

class Descriptor {
public:
    // The descriptor set layout is used by the graphics pipeline, to know how to bind the descriptor sets
    vk::raii::DescriptorSetLayout setLayout {nullptr}; // for model view projection, which uses uniform buffers

    // The descriptor pool is only used when allocating descriptor sets
    vk::raii::DescriptorPool pool {nullptr};

    void create(
        vk::raii::Device const & device,
        uint32_t const uniformBufferCount,
        uint32_t const combinedImageSamplerCount,
        uint32_t const maxSets
    );

private:
    void createDescriptorSetLayout(vk::raii::Device const & device);
    void createDescriptorPool(
        vk::raii::Device const & device,
        uint32_t const uniformBufferCount,
        uint32_t const combinedImageSamplerCount,
        uint32_t const maxSets
    );
};