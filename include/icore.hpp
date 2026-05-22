#pragma once

// Interface for the core

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

class ICore {
public:
    virtual void init1() = 0;
    virtual void init2(vk::raii::SurfaceKHR const & surface) = 0;

    virtual vk::raii::Instance const & getInstance() const = 0;
    virtual vk::raii::PhysicalDevice const & getPhysicalDevice() const = 0;
    virtual vk::raii::Device const & getDevice() const = 0;
    virtual uint32_t const getQueueFamilyIndex() const = 0;
    virtual vk::raii::Queue const & getQueue() const = 0;
};