#include "../include/buffer.hpp"

#include <iostream>

namespace Buffer {

void create(
    vk::raii::PhysicalDevice const & physicalDevice,
    vk::raii::Device const & device,
    vk::DeviceSize const bufferSize,
    vk::BufferUsageFlags const bufferUsage,
    vk::MemoryPropertyFlags const memoryProperties,
    vk::raii::Buffer & buffer,
    vk::raii::DeviceMemory & bufferMemory
) {
    vk::BufferCreateInfo const bufferCreateInfo {
        .size = bufferSize,
        .usage = bufferUsage,
        .sharingMode = vk::SharingMode::eExclusive
    };

    buffer = vk::raii::Buffer(device, bufferCreateInfo);
    
    //vk::MemoryRequirements const memoryRequirements {buffer.getMemoryRequirements()};
    vk::BufferMemoryRequirementsInfo2 const memoryRequirementsInfo {
        .buffer = *buffer
    };
    // vk::raii::Buffer doesn't have getMemoryRequirements2, so use the function from the device
    vk::MemoryRequirements2 const memoryRequirements2 {device.getBufferMemoryRequirements2(memoryRequirementsInfo)};
    vk::MemoryRequirements const memoryRequirements {memoryRequirements2.memoryRequirements};

    uint32_t const memoryTypeIndex {
        MemoryType::find(physicalDevice, memoryRequirements.memoryTypeBits, memoryProperties)
    };
    vk::MemoryAllocateInfo memoryAllocateInfo {
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = memoryTypeIndex
    };

    // MemoryAllocateFlagsInfo is only needed if the bufferUsage includes the eShaderDeviceAddress flag.
    // Declare outside the if, otherwise it will be out of scope.
    vk::MemoryAllocateFlagsInfo constexpr memoryAllocateFlagsInfo {
        .flags = vk::MemoryAllocateFlagBits::eDeviceAddress // for acceleration structures
    };
    if (bufferUsage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
        memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
    }

    bufferMemory = vk::raii::DeviceMemory(device, memoryAllocateInfo);

    vk::DeviceSize constexpr memoryOffset {0};
    buffer.bindMemory(*bufferMemory, memoryOffset);
}

void copyToBuffer(
    vk::raii::Device const & device,
    vk::raii::Queue const & queue,
    vk::raii::CommandPool const & commandPool,
    vk::raii::Buffer const & srcBuffer,
    vk::raii::Buffer const & dstBuffer,
    vk::DeviceSize const & dstOffset,
    vk::DeviceSize const bufferSize
) {
    vk::raii::CommandBuffer commandCopyBuffer {nullptr};
    SingleTimeCommands::begin(device, commandPool, commandCopyBuffer);

    vk::BufferCopy const region {
        .srcOffset = 0,
        .dstOffset = dstOffset,
        .size = bufferSize
    };

    commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, region);

    SingleTimeCommands::end(queue, commandCopyBuffer);
}

void copyToImage(
    vk::raii::Device const & device,
    vk::raii::Queue const & queue,
    vk::raii::CommandPool const & commandPool,
    vk::raii::Buffer const & buffer,
    vk::raii::Image const & image,
    uint32_t const width,
    uint32_t const height
) {
    vk::raii::CommandBuffer commandBuffer {nullptr};
    SingleTimeCommands::begin(device, commandPool, commandBuffer);

    vk::ImageSubresourceLayers constexpr imageSubresource {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .layerCount = 1
    };
    vk::Offset3D constexpr offset3D {
        .x = 0,
        .y = 0,
        .z = 0
    };
    vk::BufferImageCopy const region {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = imageSubresource,
        .imageOffset = offset3D, 
        .imageExtent = vk::Extent3D {
            .width = width,
            .height = height,
            .depth = 1
        }
    };

    commandBuffer.copyBufferToImage(*buffer, *image, vk::ImageLayout::eTransferDstOptimal, region);

    SingleTimeCommands::end(queue, commandBuffer);
}

}