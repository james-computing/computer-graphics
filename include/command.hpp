#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

class Command {
public:
    vk::raii::CommandPool pool {nullptr};
    std::vector<vk::raii::CommandBuffer> buffers;

    void create(vk::raii::Device const & device, uint32_t const queueFamilyIndex, uint32_t const commandBufferCount);

private:
    void createCommandPool(vk::raii::Device const & device, uint32_t const queueFamilyIndex);
    void createCommandBuffers(vk::raii::Device const & device, uint32_t const commandBufferCount);
};