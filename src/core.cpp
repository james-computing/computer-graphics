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

// GETTERS

vk::raii::Instance const & Core::getInstance() const {
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