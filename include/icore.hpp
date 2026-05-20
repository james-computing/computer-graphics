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

#include "../include/vertex.hpp"

class ICore {
public:
    virtual void init() = 0;
    virtual bool step() const = 0;
    virtual void cleanup() = 0;

    virtual void drawFrame(std::vector<vk::raii::DescriptorSet> const & descriptorSets, uint32_t const indexCount) = 0;

    virtual void createBuffer(
        vk::DeviceSize const bufferSize,
        vk::BufferUsageFlags const bufferUsage,
        vk::MemoryPropertyFlags const memoryProperties,
        vk::raii::Buffer & buffer,
        vk::raii::DeviceMemory & bufferMemory
    ) const = 0;

    virtual void copyVerticesToVertexBuffer(
        std::vector<Vertex> const & vertices,
        vk::DeviceSize const & dstOffset
    ) const = 0;
    virtual void copyIndicesToIndexBuffer(
        std::vector<uint32_t> const & indices,
        vk::DeviceSize const & dstOffset
    ) const = 0;

    virtual void createImage(
        uint32_t const width,
        uint32_t const height,
        uint32_t const mipLevels,
        vk::SampleCountFlagBits const numSamples,
        vk::Format const imageFormat,
        vk::ImageTiling const imageTiling,
        vk::ImageUsageFlags const imageUsage,
        vk::MemoryPropertyFlags const imageMemoryProperties,
        vk::raii::Image & image,
        vk::raii::DeviceMemory & imageMemory
    ) const = 0;

    virtual vk::raii::ImageView createImageView(
        vk::raii::Image const & image,
        vk::Format const format,
        vk::ImageAspectFlags const  aspectFlags,
        uint32_t const mipLevels
    ) const = 0;

    virtual void beginSingleTimeCommands(vk::raii::CommandBuffer & commandBuffer) const = 0;
    virtual void endSingleTimeCommands(vk::raii::CommandBuffer const & commandBuffer) const = 0;

    virtual void copyBufferToImage(
        vk::raii::Buffer const & buffer,
        vk::raii::Image const & image,
        uint32_t const width,
        uint32_t const height
    ) const = 0;

    virtual void createTextureSampler(vk::raii::Sampler & textureSampler) const = 0;

    virtual void allocateDescriptorSets(
        uint32_t const descriptorSetCount,
        std::vector<vk::raii::DescriptorSet> & descriptorSets
    ) const = 0;

    virtual void updateDescriptorSets(std::vector<vk::WriteDescriptorSet> const & writeDescriptorSets) const = 0;

    virtual uint32_t getSwapChainExtentWidth() const = 0;
    virtual uint32_t getSwapChainExtentHeight() const = 0;

    virtual uint32_t getMaxFramesInFlight() const = 0;
    virtual uint32_t getFrameIndex() const = 0;

    virtual vk::FormatProperties getFormatProperties(vk::Format const imageFormat) const = 0;
};