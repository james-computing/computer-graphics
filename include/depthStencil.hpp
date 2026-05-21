#pragma once

#include "icore.hpp"
#include "format.hpp"
#include "image.hpp"

class DepthStencil {
public:
    vk::raii::Image depthImage {nullptr};
    vk::raii::DeviceMemory depthImageMemory {nullptr};
    vk::raii::ImageView depthImageView {nullptr};
    vk::Format depthFormat;

    bool hasStencilComponent(vk::Format const format) const;
    void initDepthFormat(vk::raii::PhysicalDevice const & physicalDevice);
    void createDepthResources(
        ICore const & core,
        int const swapChainExtentWidth,
        int const swapChainExtentHeight,
        vk::SampleCountFlagBits const msaaSamples
    );
};