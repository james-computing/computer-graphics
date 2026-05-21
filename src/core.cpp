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

void Core::createBuffer(
    vk::DeviceSize const bufferSize,
    vk::BufferUsageFlags const bufferUsage,
    vk::MemoryPropertyFlags const memoryProperties,
    vk::raii::Buffer & buffer,
    vk::raii::DeviceMemory & bufferMemory
) const {
    vk::BufferCreateInfo const bufferCreateInfo {
        .size = bufferSize,
        .usage = bufferUsage,
        .sharingMode = vk::SharingMode::eExclusive
    };

    buffer = vk::raii::Buffer(device.vkraii, bufferCreateInfo);

    vk::MemoryRequirements const memoryRequirements {buffer.getMemoryRequirements()};

    uint32_t const memoryTypeIndex {
        physicalDevice.findMemoryType(
            memoryRequirements.memoryTypeBits,
            memoryProperties
        )
    };
    vk::MemoryAllocateInfo const memoryAllocateInfo {
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = memoryTypeIndex
    };

    bufferMemory = vk::raii::DeviceMemory(device.vkraii, memoryAllocateInfo);

    vk::DeviceSize constexpr memoryOffset {0};
    buffer.bindMemory(*bufferMemory, memoryOffset);
}

void Core::beginSingleTimeCommands(vk::raii::CommandBuffer & commandBuffer, vk::raii::CommandPool const & commandPool) const {
    vk::CommandBufferAllocateInfo const commandBufferAllocateInfo {
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };

    commandBuffer = std::move(device.vkraii.allocateCommandBuffers(commandBufferAllocateInfo).front());

    vk::CommandBufferBeginInfo constexpr commandBufferBeginInfo {
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    };
    commandBuffer.begin(commandBufferBeginInfo);
}

void Core::endSingleTimeCommands(vk::raii::CommandBuffer const & commandBuffer) const {
    commandBuffer.end();

    vk::SubmitInfo const submitInfo {
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffer
    };
    queue.vkraii.submit(submitInfo, {});
    queue.vkraii.waitIdle();
}

void Core::copyBuffer(
    vk::raii::Buffer const & srcBuffer,
    vk::raii::Buffer const & dstBuffer,
    vk::DeviceSize const & dstOffset,
    vk::DeviceSize const bufferSize,
    vk::raii::CommandPool const & commandPool
) const {
    vk::raii::CommandBuffer commandCopyBuffer {nullptr};
    beginSingleTimeCommands(commandCopyBuffer, commandPool);

    vk::BufferCopy const region {
        .srcOffset = 0,
        .dstOffset = dstOffset,
        .size = bufferSize
    };

    commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, region);

    endSingleTimeCommands(commandCopyBuffer);
}

void Core::createImage(
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
) const {
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

    image = vk::raii::Image(device.vkraii, imageCreateInfo);

    // Allocate memory for the image
    vk::MemoryRequirements const memoryRequirements {image.getMemoryRequirements()};
    vk::MemoryAllocateInfo const memoryAllocateInfo {
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = physicalDevice.findMemoryType(memoryRequirements.memoryTypeBits, imageMemoryProperties)
    };
    imageMemory = vk::raii::DeviceMemory(device.vkraii, memoryAllocateInfo);
    // Bind the memory
    image.bindMemory(imageMemory, 0);
}

vk::raii::ImageView Core::createImageView(
    vk::raii::Image const & image,
    vk::Format const format,
    vk::ImageAspectFlags const aspectFlags,
    uint32_t const mipLevels
) const {
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

    return vk::raii::ImageView(device.vkraii, imageViewCreateInfo);
}

void Core::copyBufferToImage(
    vk::raii::Buffer const & buffer,
    vk::raii::Image const & image,
    uint32_t const width,
    uint32_t const height,
    vk::raii::CommandPool const & commandPool
) const {
    vk::raii::CommandBuffer commandBuffer {nullptr};
    beginSingleTimeCommands(commandBuffer, commandPool);


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

    endSingleTimeCommands(commandBuffer);
}

// TEXTURE SAMPLER

void Core::createTextureSampler(vk::raii::Sampler & textureSampler) const {
    vk::PhysicalDeviceProperties physicalDeviceProperties {physicalDevice.vkraii.getProperties()};

    vk::SamplerCreateInfo const samplerCreateInfo {
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .mipLodBias = 0.0f,
        .anisotropyEnable = vk::True,
        .maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy,
        .compareEnable = vk::False,
        .compareOp = vk::CompareOp::eAlways,
        .minLod = 0.0f,
        .maxLod = vk::LodClampNone,
        .borderColor = vk::BorderColor::eIntOpaqueBlack,
        .unnormalizedCoordinates = vk::False
    };

    textureSampler = vk::raii::Sampler(device.vkraii, samplerCreateInfo);
}

// GETTERS

vk::raii::Instance const & Core::getInstance() {
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