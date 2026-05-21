#include "../include/format.hpp"

vk::Format Format::findSupportedFormat(
    std::vector<vk::Format> const & candidateFormats,
    vk::ImageTiling const tiling,
    vk::FormatFeatureFlags const features,
    vk::raii::PhysicalDevice const & physicalDevice
) {
    switch (tiling) {
        case vk::ImageTiling::eLinear:
            for (vk::Format const & format : candidateFormats) {
                vk::FormatProperties const props {physicalDevice.getFormatProperties(format)};
                if ((props.linearTilingFeatures & features) == features) {
                    return format;
                }
            }
            break;
        case vk::ImageTiling::eOptimal:
            for (vk::Format const & format : candidateFormats) {
                vk::FormatProperties const props {physicalDevice.getFormatProperties(format)};
                if ((props.optimalTilingFeatures & features) == features) {
                    return format;
                }
            }
            break;
    }
    throw std::runtime_error("Failed to find supported format");
}