#include "../include/physicalDevice.hpp"

bool PhysicalDevice::isSuitable(vk::raii::PhysicalDevice const & physicalDevice) {
    vk::PhysicalDeviceProperties const deviceProperties {physicalDevice.getProperties()};
    vk::PhysicalDeviceFeatures const deviceFeatures {physicalDevice.getFeatures()};
    std::vector<vk::QueueFamilyProperties> const queueFamilies {physicalDevice.getQueueFamilyProperties()};
    std::vector<char const *> const requiredDeviceExtensions({vk::KHRSwapchainExtensionName});

    bool const supportsVulkan1_3 {deviceProperties.apiVersion >= vk::ApiVersion13};

    bool const supportsGraphics {
        std::ranges::any_of(
            queueFamilies,
            [] (vk::QueueFamilyProperties const &qfp) -> bool {
                return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
            }
        )
    };

    // try catch?
    std::vector<vk::ExtensionProperties> const availableDeviceExtensions {physicalDevice.enumerateDeviceExtensionProperties()};

    bool const supportsAllRequiredExtensions {
        // All of the required device extensions are any of the available extensions
        std::ranges::all_of(
            requiredDeviceExtensions,
            [&availableDeviceExtensions] (char const * const & requiredDeviceExtension) -> bool {
                return std::ranges::any_of(
                    availableDeviceExtensions,
                    [requiredDeviceExtension] (vk::ExtensionProperties const & availableDeviceExtension) -> bool {
                        return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0;
                    }
                );
            }
        )
    };

    auto const features {
        // .template is to tell the compiler to use the method that comes from templates, avoiding ambiguity
        physicalDevice.template getFeatures2<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceVulkan11Features, // for shader module creation
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
        >()
    };
    bool const supportsRequiredFeatures {
        
        features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters && // for shader module creation
        features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
        features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
        features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState &&
        features.template get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy // for texture sampler
    };

    return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;

    /*
    if (
        deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
        deviceFeatures.geometryShader &&
        supportsVulkan1_3
    ) {
        return true;
    }

    return false;*/
}

void PhysicalDevice::pick(vk::raii::Instance const & instance) {
    // try catch?
    std::vector<vk::raii::PhysicalDevice> const physicalDevices {instance.enumeratePhysicalDevices()};
    
    auto const deviceIterator {
        // Find if there is some suitable device
        std::ranges::find_if(
            physicalDevices,
            [this] (vk::raii::PhysicalDevice const & physicalDevice) -> bool {
                return isSuitable(physicalDevice);
            }
        )
    };

    if (deviceIterator == physicalDevices.end()) {
        throw std::runtime_error("Failed to find a suitable physical device");
    }

    // Pick the first suitable physical device found
    vkraii = *deviceIterator;
}

uint32_t PhysicalDevice::findMemoryType(uint32_t const typeFilter, vk::MemoryPropertyFlags const properties) const {
    vk::PhysicalDeviceMemoryProperties const memoryProperties {vkraii.getMemoryProperties()};

    for (uint32_t i {0}; i < memoryProperties.memoryTypeCount; ++i) {
        if (
            (typeFilter & (1 << i)) &&
            (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties
        ) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
}

vk::Format PhysicalDevice::findSupportedFormat(
    std::vector<vk::Format> const & candidateFormats,
    vk::ImageTiling const tiling,
    vk::FormatFeatureFlags const features
) const {
    switch (tiling) {
        case vk::ImageTiling::eLinear:
            for (vk::Format const & format : candidateFormats) {
                vk::FormatProperties const props {vkraii.getFormatProperties(format)};
                if ((props.linearTilingFeatures & features) == features) {
                    return format;
                }
            }
            break;
        case vk::ImageTiling::eOptimal:
            for (vk::Format const & format : candidateFormats) {
                vk::FormatProperties const props {vkraii.getFormatProperties(format)};
                if ((props.optimalTilingFeatures & features) == features) {
                    return format;
                }
            }
            break;
    }
    throw std::runtime_error("Failed to find supported format");
}