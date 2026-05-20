#pragma once

#include "../include/icore.hpp"

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
#include "../include/graphicsPipeline.hpp"
#include "../include/vertex.hpp"
#include "../include/mvp.hpp"
#include "../include/depthStencil.hpp"
#include "../include/msaa.hpp"
#include "../include/vertexBuffer.hpp"
#include "../include/indexBuffer.hpp"

#include "../libraries/stb/stb_image.h"

// Used in drawFrame
struct SyncObjects {
    std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Fence> inFlightFences;
};

class Core : public ICore {
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
    GraphicsPipeline graphicsPipeline;
    
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
    void init() override;
    bool step() const override;
    void cleanup() override;

    void drawFrame(std::vector<vk::raii::DescriptorSet> const & descriptorSets, uint32_t const indexCount) override;

    void createBuffer(
        vk::DeviceSize const bufferSize,
        vk::BufferUsageFlags const bufferUsage,
        vk::MemoryPropertyFlags const memoryProperties,
        vk::raii::Buffer & buffer,
        vk::raii::DeviceMemory & bufferMemory
    ) const override;

    void copyVerticesToVertexBuffer(
        std::vector<Vertex> const & vertices,
        vk::DeviceSize const & dstOffset
    ) const override;
    void copyIndicesToIndexBuffer(
        std::vector<uint32_t> const & indices,
        vk::DeviceSize const & dstOffset
    ) const override;

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
    ) const override;

    vk::raii::ImageView createImageView(
        vk::raii::Image const & image,
        vk::Format const format,
        vk::ImageAspectFlags const  aspectFlags,
        uint32_t const mipLevels
    ) const override;

    void beginSingleTimeCommands(vk::raii::CommandBuffer & commandBuffer) const override;
    void endSingleTimeCommands(vk::raii::CommandBuffer const & commandBuffer) const override;

    void copyBufferToImage(
        vk::raii::Buffer const & buffer,
        vk::raii::Image const & image,
        uint32_t const width,
        uint32_t const height
    ) const override;

    void createTextureSampler(vk::raii::Sampler & textureSampler) const override;

    void allocateDescriptorSets(
        uint32_t const descriptorSetCount,
        std::vector<vk::raii::DescriptorSet> & descriptorSets
    ) const override;

    void updateDescriptorSets(std::vector<vk::WriteDescriptorSet> const & writeDescriptorSets) const override;

    uint32_t getSwapChainExtentWidth() const override;
    uint32_t getSwapChainExtentHeight() const override;

    uint32_t getMaxFramesInFlight() const override;
    uint32_t getFrameIndex() const override;

    vk::FormatProperties getFormatProperties(vk::Format const imageFormat) const override;

private:
    void initVulkan();

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

    void copyBuffer(
        vk::raii::Buffer const & srcBuffer,
        vk::raii::Buffer const & dstBuffer,
        vk::DeviceSize const & dstOffset,
        vk::DeviceSize const bufferSize
    ) const;

    template <typename T>
    void copyToBuffer(
        std::vector<T> const & v,
        vk::DeviceSize const & dstOffset,
        vk::raii::Buffer const & buffer
    ) const;

    void createDescriptorSetLayout();
    void createDescriptorPool();

    // Group swap chain recreation with color and depth resources recreation
    void recreateSwapChainColorDepth();
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