#include "../include/indexBuffer.hpp"

void IndexBuffer::create(ICore const & core) {
    // Size of both staging and index buffers
    vk::DeviceSize const bufferSize {MAX_INDICES * sizeof(uint32_t)};

    // Create the index buffer
    vk::BufferUsageFlags constexpr indexbufferUsage {vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst};
    vk::MemoryPropertyFlags constexpr indexBufferMemoryProperties {vk::MemoryPropertyFlagBits::eDeviceLocal};
    Buffer::create(
        core.getPhysicalDevice(),
        core.getDevice(),
        bufferSize,
        indexbufferUsage,
        indexBufferMemoryProperties,
        buffer,
        memory
    );
}