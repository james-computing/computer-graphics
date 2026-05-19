#include "../include/device.hpp"

void Device::create(vk::raii::PhysicalDevice const & physicalDevice, vk::raii::SurfaceKHR const & surface, Queue & queue) {
    std::vector<vk::QueueFamilyProperties> const queueFamilyProperties {
        physicalDevice.getQueueFamilyProperties()
    };

    // Find first queue with graphics support which is also capable of presenting to the window,
    // and store its index.
    bool foundSuitableQueue {false};
    queue.index = 0;
    size_t const queueFamilyPropertiesSize {queueFamilyProperties.size()};
    for (; queue.index < queueFamilyPropertiesSize; ++queue.index) {
        bool supportsGraphics = (queueFamilyProperties[queue.index].queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
        
        // try catch?
        bool supportsWindowPresentation = physicalDevice.getSurfaceSupportKHR(queue.index, *surface);

        if (supportsGraphics && supportsWindowPresentation) {
            foundSuitableQueue = true;
            break;
        }
    }

    if (!foundSuitableQueue) {
        throw std::runtime_error("Failed to find suitable queue");
    }

    float constexpr queuePriority {0.5f};
    vk::DeviceQueueCreateInfo const deviceQueueCreateInfo {
        .queueFamilyIndex = queue.index,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    //vk::PhysicalDeviceFeatures constexpr deviceFeatures;

    // Create a chain of featured structures.
    // Vulkan uses multiple features by chaining the features and then passing the first feature of the chain.
    // In C, the chain is constructed using the pNext property.
    vk::StructureChain<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
        vk::PhysicalDeviceVulkan11Features
    > const featureChain {
        {.features = {.samplerAnisotropy = true }},
        {
            .synchronization2 = true, // sync objects
            .dynamicRendering = true
        },
        {.extendedDynamicState = true},
        {.shaderDrawParameters = true} // for shader module creation
    };

    std::vector<char const *> const requiredDeviceExtensions {
        vk::KHRSwapchainExtensionName
    };

    vk::DeviceCreateInfo const deviceCreateInfo {
        .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &deviceQueueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size()),
        .ppEnabledExtensionNames = requiredDeviceExtensions.data()
    };

    // try catch?
    vkraii = vk::raii::Device(physicalDevice, deviceCreateInfo);

    queue.vkraii = vk::raii::Queue(vkraii, queue.index, 0);
}