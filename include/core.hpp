#pragma once

#include "../include/icore.hpp"

#include "../include/validationLayers.hpp"
#include "../include/debugMessenger.hpp"
#include "../include/instance.hpp"
#include "../include/physicalDevice.hpp"
#include "../include/queue.hpp"
#include "../include/device.hpp"

class Core : public ICore {
private:
    ///////////////////////////////////////////////// MEMBER VARIABLES //////////////////////////////////

    vk::raii::Context context;
    Instance instance;
    ValidationLayers validationLayers;
    DebugMessenger debugMessenger;
    PhysicalDevice physicalDevice;
    Device device;
    Queue queue;

    /////////////////////////////////////// METHODS //////////////////////////////////////////////////
public:
    void init1() override;
    void init2(vk::raii::SurfaceKHR const & surface) override;
    
    void createBuffer(
        vk::DeviceSize const bufferSize,
        vk::BufferUsageFlags const bufferUsage,
        vk::MemoryPropertyFlags const memoryProperties,
        vk::raii::Buffer & buffer,
        vk::raii::DeviceMemory & bufferMemory
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

    void beginSingleTimeCommands(vk::raii::CommandBuffer & commandBuffer, vk::raii::CommandPool const & commandPool) const override;
    void endSingleTimeCommands(vk::raii::CommandBuffer const & commandBuffer) const override;

    void copyBuffer(
        vk::raii::Buffer const & srcBuffer,
        vk::raii::Buffer const & dstBuffer,
        vk::DeviceSize const & dstOffset,
        vk::DeviceSize const bufferSize,
        vk::raii::CommandPool const & commandPool
    ) const;

    void copyBufferToImage(
        vk::raii::Buffer const & buffer,
        vk::raii::Image const & image,
        uint32_t const width,
        uint32_t const height,
        vk::raii::CommandPool const & commandPool
    ) const override;

    vk::raii::Instance const & getInstance() override;
    vk::raii::PhysicalDevice const & getPhysicalDevice() const override;
    vk::raii::Device const & getDevice() const override;
    uint32_t const getQueueFamilyIndex() const override;
    vk::raii::Queue const & getQueue() const override;
};