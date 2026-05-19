#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include "queue.hpp"

class Device {
public:
    vk::raii::Device vkraii {nullptr}; // logical device

    void create(vk::raii::PhysicalDevice const & physicalDevice, vk::raii::SurfaceKHR const & surface, Queue & queue);
};