#include "../include/msaa.hpp"

// MSAA

void MSAA::initMaxUsableSampleCount(vk::raii::PhysicalDevice const & physicalDevice) {
    vk::PhysicalDeviceProperties const physicalDeviceProperties {physicalDevice.getProperties()};

    vk::SampleCountFlags sampleCounts {
        physicalDeviceProperties.limits.framebufferColorSampleCounts &
        physicalDeviceProperties.limits.framebufferDepthSampleCounts
    };

    if (sampleCounts & vk::SampleCountFlagBits::e64) {
        samples = vk::SampleCountFlagBits::e64;
        return;
    }
    if (sampleCounts & vk::SampleCountFlagBits::e32) {
        samples = vk::SampleCountFlagBits::e32;
        return;
    }
    if (sampleCounts & vk::SampleCountFlagBits::e16) {
        samples = vk::SampleCountFlagBits::e16;
        return;
    }
    if (sampleCounts & vk::SampleCountFlagBits::e8) {
        samples = vk::SampleCountFlagBits::e8;
        return;
    }
    if (sampleCounts & vk::SampleCountFlagBits::e4) {
        samples = vk::SampleCountFlagBits::e4;
        return;
    }
    if (sampleCounts & vk::SampleCountFlagBits::e2) {
        samples = vk::SampleCountFlagBits::e2;
        return;
    }

    samples = vk::SampleCountFlagBits::e1;
}

void MSAA::createColorResources(
    ICore const & core,
    vk::Format const colorFormat,
    int const swapChainExtentWidth,
    int const swapChainExtentHeight
) {
    Image::create(
        core.getPhysicalDevice(),
        core.getDevice(),
        swapChainExtentWidth,
        swapChainExtentHeight,
        1,
        samples,
        colorFormat,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        colorImage,
        colorImageMemory
    );

    colorImageView = Image::createView(core.getDevice(), colorImage, colorFormat, vk::ImageAspectFlagBits::eColor, 1);
}