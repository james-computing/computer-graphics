#pragma once

#include "icore.hpp"
#include "physicalDevice.hpp"

class DepthStencil {
public:
    vk::raii::Image depthImage {nullptr};
    vk::raii::DeviceMemory depthImageMemory {nullptr};
    vk::raii::ImageView depthImageView {nullptr};
    vk::Format depthFormat;

    bool hasStencilComponent(vk::Format const format) const;
    void initDepthFormat(PhysicalDevice const & physicalDevice);
    void createDepthResources(ICore const & core, vk::SampleCountFlagBits msaaSamples);
};