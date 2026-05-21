#include "../include/memoryType.hpp"

namespace MemoryType {

uint32_t find(vk::raii::PhysicalDevice const & physicaldevice, uint32_t const typeFilter, vk::MemoryPropertyFlags const properties) {
    vk::PhysicalDeviceMemoryProperties const memoryProperties {physicaldevice.getMemoryProperties()};

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

}