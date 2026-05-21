#include "../include/command.hpp"

void Command::createCommandPool(vk::raii::Device const & device, uint32_t const queueFamilyIndex) {
    vk::CommandPoolCreateInfo const commandPoolCreateInfo {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = queueFamilyIndex
    };

    pool = vk::raii::CommandPool(device, commandPoolCreateInfo);
}

void Command::createCommandBuffers(vk::raii::Device const & device, uint32_t const commandBufferCount) {
    vk::CommandBufferAllocateInfo const commandBufferAllocateInfo {
        .commandPool = pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = commandBufferCount
    };

    // vk::raii::CommandBuffers inherits from std::vector<vk::raii:CommandBuffer>
    buffers = vk::raii::CommandBuffers(device, commandBufferAllocateInfo);
}

void Command::create(vk::raii::Device const & device, uint32_t const queueFamilyIndex, uint32_t const commandBufferCount) {
    createCommandPool(device, queueFamilyIndex);
    createCommandBuffers(device, commandBufferCount);
}