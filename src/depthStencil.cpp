#include "../include/depthStencil.hpp"

// DEPTH & STENCIL

bool DepthStencil::hasStencilComponent(vk::Format const format) const {
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

void DepthStencil::initDepthFormat(PhysicalDevice const & physicalDevice) {
    std::vector<vk::Format> const candidateFormats {
        vk::Format::eD32Sfloat,
        vk::Format::eD32SfloatS8Uint,
        vk::Format::eD24UnormS8Uint
    };

    depthFormat = physicalDevice.findSupportedFormat(
        candidateFormats,
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}

void DepthStencil::createDepthResources(ICore const & core, vk::SampleCountFlagBits msaaSamples) {
    // Create depth image, allocate memory for it and bind it
    core.createImage(
        core.getSwapChainExtentWidth(),
        core.getSwapChainExtentHeight(),
        1,
        msaaSamples,
        depthFormat,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        depthImage,
        depthImageMemory
    );

    // Create depth image view
    depthImageView = core.createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, 1);
}