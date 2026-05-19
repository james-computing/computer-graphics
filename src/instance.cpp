#include "../include/instance.hpp"

void Instance::create(vk::raii::Context const & context, ValidationLayers const & validationLayers) {
    std::vector<char const *> const requiredGLFWExtensions = GLFWExtensions::getRequiredGLFWExtensions(context, validationLayers.enable);
    std::vector<char const *> const requiredValidationLayers = validationLayers.getRequiredValidationLayers(context);

    vk::ApplicationInfo constexpr appInfo {
        .pApplicationName = "Application",
        .applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0), // VK_MAKE_VERSION is deprecated
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0), // VK_MAKE_VERSION is deprecated
        .apiVersion = vk::ApiVersion14
    };

    vk::InstanceCreateInfo const createInfo {
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requiredValidationLayers.size()),
        .ppEnabledLayerNames = requiredValidationLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(requiredGLFWExtensions.size()),
        .ppEnabledExtensionNames = requiredGLFWExtensions.data()
    };

    // try catch?
    vkraii = vk::raii::Instance(context, createInfo);
}