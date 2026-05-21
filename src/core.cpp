#include "../include/core.hpp"

void Core::init1() {
    std::cout << "init instance" << std::endl;
    instance.create(context, validationLayers);

    // depends on instance
    if (validationLayers.enable) {
        debugMessenger.setup(instance.vkraii); // make debug messenger first, because we want to be able to debug early
    }
    std::cout << "init physical device" << std::endl;
    physicalDevice.pick(instance.vkraii);
}

void Core::init2(vk::raii::SurfaceKHR const & surface) {
    // depends on physicalDevice and surface
    std::cout << "init logical device" << std::endl;
    device.create(physicalDevice.vkraii, surface, queue);
}

void Core::copyBufferToImage(
    vk::raii::Buffer const & buffer,
    vk::raii::Image const & image,
    uint32_t const width,
    uint32_t const height,
    vk::raii::Device const & device,
    vk::raii::Queue const & queue,
    vk::raii::CommandPool const & commandPool
) const {
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

// GETTERS

vk::raii::Instance const & Core::getInstance() const {
    return instance.vkraii;
}

vk::raii::PhysicalDevice const & Core::getPhysicalDevice() const {
    return physicalDevice.vkraii;
}

vk::raii::Device const & Core::getDevice() const {
    return device.vkraii;
}

uint32_t const Core::getQueueFamilyIndex() const {
    return queue.familyIndex;
}

vk::raii::Queue const & Core::getQueue() const {
    return queue.vkraii;
}