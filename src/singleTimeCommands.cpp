#include "../include/singleTimeCommands.hpp"

namespace SingleTimeCommands {

void begin(vk::raii::Device const & device, vk::raii::CommandPool const & commandPool, vk::raii::CommandBuffer & commandBuffer) {
    vk::CommandBufferAllocateInfo const commandBufferAllocateInfo {
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };

    commandBuffer = std::move(device.allocateCommandBuffers(commandBufferAllocateInfo).front());

    vk::CommandBufferBeginInfo constexpr commandBufferBeginInfo {
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    };
    commandBuffer.begin(commandBufferBeginInfo);
}

void end(vk::raii::Queue const & queue, vk::raii::CommandBuffer const & commandBuffer) {
    commandBuffer.end();

    vk::SubmitInfo const submitInfo {
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffer
    };
    queue.submit(submitInfo, {});
    queue.waitIdle();
}

}