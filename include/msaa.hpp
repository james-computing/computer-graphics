#pragma once

#include "icore.hpp"
#include "image.hpp"

class MSAA {
public:
    vk::SampleCountFlagBits samples {vk::SampleCountFlagBits::e1};
    vk::raii::Image colorImage {nullptr};
    vk::raii::DeviceMemory colorImageMemory {nullptr};
    vk::raii::ImageView colorImageView {nullptr};

    void initMaxUsableSampleCount(vk::raii::PhysicalDevice const & physicalDevice);
    void createColorResources(
        ICore const & core,
        vk::Format const colorFormat,
        int const swapChainExtentWidth,
        int const swapChainExtentHeight
    );
};