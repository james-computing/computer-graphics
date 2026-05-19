#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

class PhysicalDevice {
public:
    vk::raii::PhysicalDevice vkraii {nullptr};
    void pick(vk::raii::Instance const & instance);
    uint32_t findMemoryType(uint32_t const typeFilter, vk::MemoryPropertyFlags const properties) const;

private:
    static bool isSuitable(vk::raii::PhysicalDevice const & physicalDevice);
};