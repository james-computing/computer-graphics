#include "../include/image.hpp"

namespace Image {

void create(
    vk::raii::PhysicalDevice const & physicalDevice,
    vk::raii::Device const & device,
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
) {
    vk::Extent3D const extent {
        .width = width,
        .height = height,
        .depth = 1
    };
    vk::ImageCreateInfo const imageCreateInfo {
        .imageType = vk::ImageType::e2D,
        .format = imageFormat,
        .extent = extent,
        .mipLevels = mipLevels,
        .arrayLayers = 1,
        .samples = numSamples,
        .tiling = imageTiling,
        .usage = imageUsage,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined
    };

    image = vk::raii::Image(device, imageCreateInfo);

    // Allocate memory for the image
    vk::MemoryRequirements const memoryRequirements {image.getMemoryRequirements()};
    vk::MemoryAllocateInfo const memoryAllocateInfo {
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = MemoryType::find(physicalDevice, memoryRequirements.memoryTypeBits, imageMemoryProperties)
    };
    imageMemory = vk::raii::DeviceMemory(device, memoryAllocateInfo);
    // Bind the memory
    image.bindMemory(imageMemory, 0);
}

vk::raii::ImageView createView(
    vk::raii::Device const & device,
    vk::raii::Image const & image,
    vk::Format const format,
    vk::ImageAspectFlags const aspectFlags,
    uint32_t const mipLevels
) {
    vk::ImageViewCreateInfo const imageViewCreateInfo {
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .subresourceRange = vk::ImageSubresourceRange {
            .aspectMask = aspectFlags,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    return vk::raii::ImageView(device, imageViewCreateInfo);
}

}