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

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../include/surface.hpp"
#include "../include/vertex.hpp"

class ICore {
public:
    // glfwWindow can't be const * const, because of the function glfwCreateWindowSurface 
    virtual void init1() = 0;
    virtual void init2(vk::raii::SurfaceKHR const & surface) = 0;

    virtual void copyBufferToImage(
        vk::raii::Buffer const & buffer,
        vk::raii::Image const & image,
        uint32_t const width,
        uint32_t const height,
        vk::raii::Device const & device,
        vk::raii::Queue const & queue,
        vk::raii::CommandPool const & commandPool
    ) const = 0;

    virtual vk::raii::Instance const & getInstance() const = 0;
    virtual vk::raii::PhysicalDevice const & getPhysicalDevice() const = 0;
    virtual vk::raii::Device const & getDevice() const = 0;
    virtual uint32_t const getQueueFamilyIndex() const = 0;
    virtual vk::raii::Queue const & getQueue() const = 0;
};