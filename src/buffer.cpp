#include "../include/buffer.hpp"

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

    vk::MemoryRequirements const memoryRequirements {buffer.getMemoryRequirements()};

    uint32_t const memoryTypeIndex {
        MemoryType::find(physicalDevice, memoryRequirements.memoryTypeBits, memoryProperties)
    };
    vk::MemoryAllocateInfo const memoryAllocateInfo {
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = memoryTypeIndex
    };

    bufferMemory = vk::raii::DeviceMemory(device, memoryAllocateInfo);

    vk::DeviceSize constexpr memoryOffset {0};
    buffer.bindMemory(*bufferMemory, memoryOffset);
}

void copy(
    vk::raii::Buffer const & srcBuffer,
    vk::raii::Buffer const & dstBuffer,
    vk::DeviceSize const & dstOffset,
    vk::DeviceSize const bufferSize,
    vk::raii::Device const & device,
    vk::raii::Queue const & queue,
    vk::raii::CommandPool const & commandPool
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