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
#include <stdexcept>
#include <cstdint> // For uint32_t
#include <limits> // for std::numeric_limits
#include <algorithm> // for std::clamp
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/gtc/matrix_transform.hpp> // for model view projection
#include <chrono> // for model view projection
#include <unordered_map>

#include "../include/validationLayers.hpp"
#include "../include/debugMessenger.hpp"
#include "../include/window.hpp"
#include "../include/surface.hpp"
#include "../include/instance.hpp"
#include "../include/physicalDevice.hpp"
#include "../include/queue.hpp"
#include "../include/device.hpp"
#include "../include/swapChain.hpp"
#include "../include/vertex.hpp"
#include "../include/mvp.hpp"
#include "../include/shader.hpp"

#include "../libraries/stb/stb_image.h"

struct DepthStencil {
    vk::raii::Image depthImage {nullptr};
    vk::raii::DeviceMemory depthImageMemory {nullptr};
    vk::raii::ImageView depthImageView {nullptr};
    vk::Format depthFormat;
};

struct MSAA {
    vk::SampleCountFlagBits samples {vk::SampleCountFlagBits::e1};
    vk::raii::Image colorImage {nullptr};
    vk::raii::DeviceMemory colorImageMemory {nullptr};
    vk::raii::ImageView colorImageView {nullptr};
};

struct VertexBuffer {
    vk::raii::Buffer buffer {nullptr};
    vk::raii::DeviceMemory memory {nullptr};
    size_t const MAX_VERTICES {4000};
};

struct IndexBuffer {
    vk::raii::Buffer buffer {nullptr};
    vk::raii::DeviceMemory memory {nullptr};
    size_t const MAX_INDICES {12000};
    uint32_t numIndices {0};
};

struct SyncObjects {
    std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Fence> inFlightFences;
};

class Core {
private:
    ///////////////////////////////////////////////// MEMBER VARIABLES //////////////////////////////////
    Window window;

    vk::raii::Context context;

    Instance instance;

    ValidationLayers validationLayers;
    DebugMessenger debugMessenger;

    PhysicalDevice physicalDevice;
    Device device;

    Queue queue;

    Surface surface;
    SwapChain swapChain;

    vk::raii::PipelineLayout pipelineLayout {nullptr};
    vk::raii::Pipeline graphicsPipeline {nullptr};

    vk::raii::CommandPool commandPool {nullptr};
    std::vector<vk::raii::CommandBuffer> commandBuffers;

    SyncObjects syncObjects;

    uint32_t const MAX_FRAMES_IN_FLIGHT {2};
    uint32_t frameIndex {0};

    VertexBuffer vertexBuffer;
    IndexBuffer indexBuffer;

    vk::raii::DescriptorSetLayout descriptorSetLayout {nullptr}; // for model view projection, which uses uniform buffers
    vk::raii::DescriptorPool descriptorPool {nullptr};

    DepthStencil depthStencil;
    // multisampling
    MSAA msaa;

    /////////////////////////////////////// METHODS //////////////////////////////////////////////////
public:
    void init();
    bool step() const;
    void cleanup();

    void drawFrame(std::vector<vk::raii::DescriptorSet> const & descriptorSets, uint32_t const indexCount);

    void createBuffer(
        vk::DeviceSize const bufferSize,
        vk::BufferUsageFlags const bufferUsage,
        vk::MemoryPropertyFlags const memoryProperties,
        vk::raii::Buffer & buffer,
        vk::raii::DeviceMemory & bufferMemory
    ) const;

    void copyBuffer(
        vk::raii::Buffer const & srcBuffer,
        vk::raii::Buffer const & dstBuffer,
        vk::DeviceSize const & dstOffset,
        vk::DeviceSize const bufferSize
    ) const;

    void copyVerticesToVertexBuffer(
        std::vector<Vertex> const & vertices,
        vk::DeviceSize const & dstOffset
    ) const;
    void copyIndicesToIndexBuffer(
        std::vector<uint32_t> const & indices,
        vk::DeviceSize const & dstOffset
    ) const;

    void createImage(
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
    ) const;

    void beginSingleTimeCommands(vk::raii::CommandBuffer & commandBuffer) const;
    void endSingleTimeCommands(vk::raii::CommandBuffer const & commandBuffer) const;

    void copyBufferToImage(
        vk::raii::Buffer const & buffer,
        vk::raii::Image const & image,
        uint32_t const width,
        uint32_t const height
    ) const;

    vk::raii::ImageView createImageView(
        vk::raii::Image const & image,
        vk::Format const format,
        vk::ImageAspectFlags const  aspectFlags,
        uint32_t const mipLevels
    ) const;

    void createTextureSampler(vk::raii::Sampler & textureSampler) const;

    void allocateDescriptorSets(
        uint32_t const descriptorSetCount,
        std::vector<vk::raii::DescriptorSet> & descriptorSets
    ) const;

    void updateDescriptorSets(std::vector<vk::WriteDescriptorSet> const & writeDescriptorSets) const;

    uint32_t getSwapChainExtentWidth() const;
    uint32_t getSwapChainExtentHeight() const;

    uint32_t getMaxFramesInFlight() const;
    uint32_t getFrameIndex() const;

    vk::FormatProperties getFormatProperties(vk::Format const imageFormat) const;

private:
    void initVulkan();

    void createGraphicsPipeline();

    void createCommandPool();
    void createCommandBuffers();

    void transitionImageLayout(
        vk::Image const & image, // not vk::raii::Image, because swapChain.getImages returns vk::Image
        vk::ImageLayout const oldLayout,
        vk::ImageLayout const newLayout,
        vk::AccessFlags2 const srcAccessMask,
        vk::AccessFlags2 const dstAccessMask,
        vk::PipelineStageFlags2 const srcStageMask,
        vk::PipelineStageFlags2 const dstStageMask,
        vk::ImageAspectFlags const imageAspectFlags
    ) const;

    void recordCommandBuffer(
        uint32_t const imageIndex,
        std::vector<vk::raii::DescriptorSet> const & descriptorSets,
        uint32_t const indexCount
    ) const;

    void createSyncObjects();

    template <typename T>
    void copyToBuffer(
        std::vector<T> const & v,
        vk::DeviceSize const & dstOffset,
        vk::raii::Buffer const & buffer
    ) const;
    
    void createVertexBuffer();
    void createIndexBuffer();

    void createDescriptorSetLayout();
    void createDescriptorPool();

    // Depth stencil
    bool hasStencilComponent(vk::Format const format) const;
    void initDepthFormat();
    void createDepthResources();

    // MSAA
    void initMaxUsableSampleCount();
    void createColorResources();
};

// Templates must be in the header file
template <typename T>
void Core::copyToBuffer(
    std::vector<T> const & v,
    vk::DeviceSize const & dstOffset,
    vk::raii::Buffer const & buffer
) const {
    vk::DeviceSize bufferSize {v.size() * sizeof(T)};

    // Create a staging buffer to transfer data from the host to the device
    vk::BufferUsageFlags constexpr stagingBufferUsage {vk::BufferUsageFlagBits::eTransferSrc};
    vk::MemoryPropertyFlags constexpr stagingBufferMemoryProperties {
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    };
    vk::raii::Buffer stagingBuffer {nullptr};
    vk::raii::DeviceMemory stagingBufferMemory {nullptr};
    createBuffer(
        bufferSize,
        stagingBufferUsage,
        stagingBufferMemoryProperties,
        stagingBuffer,
        stagingBufferMemory
    );

    // Copy the data from the vertices vector to the staging buffer memory
    void * data {stagingBufferMemory.mapMemory(0, bufferSize)};
    memcpy(data, v.data(), bufferSize);
    stagingBufferMemory.unmapMemory();
    data = nullptr;

    // Copy data from staging buffer to vertex buffer
    copyBuffer(stagingBuffer, buffer, dstOffset, bufferSize);
}