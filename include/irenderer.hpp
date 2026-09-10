#pragma once

// Interface for the renderer

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

#include "../include/icore.hpp"
#include "../include/vertex.hpp"

class IRenderer {
public:
    virtual void initWindow() = 0;
    virtual void initSurface(vk::raii::Instance const & instance) = 0;
    virtual void initRest(ICore const & core) = 0;
    virtual bool step() const = 0;
    virtual void cleanup() = 0;

    virtual void drawFrame(uint32_t const indexCount, uint32_t const instanceCount) = 0;

    virtual void copyVerticesToVertexBuffer(
        std::vector<Vertex> const & vertices,
        size_t const & offset
    ) const = 0;
    virtual void copyIndicesToIndexBuffer(
        std::vector<uint32_t> const & indices,
        size_t const & offset
    ) const = 0;

    virtual vk::raii::SurfaceKHR const & getSurface() const = 0;

    virtual uint32_t getSwapChainExtentWidth() const = 0;
    virtual uint32_t getSwapChainExtentHeight() const = 0;

    virtual uint32_t getMaxFramesInFlight() const = 0;
    virtual uint32_t getFrameIndex() const = 0;

    virtual vk::raii::CommandPool const & getCommandPool() const = 0;

    virtual void updateDescriptorSets(
        vk::raii::ImageView const & textureImageView,
        std::vector<vk::raii::Buffer> const & cameraUniformBuffers,
        //std::vector<vk::raii::Buffer> const & objectUniformBuffers
        std::vector<vk::raii::Buffer> const & modelInstancesSSBOs,
        uint32_t const instanceCount
    ) const = 0;
};