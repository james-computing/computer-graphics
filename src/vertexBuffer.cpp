#include "../include/vertexBuffer.hpp"

void VertexBuffer::create(ICore const & core) {
    // Should change the buffer size to something else, but still with enough space for the vertex data.
    // Size of both staging and vertex buffers
    vk::DeviceSize const bufferSize {MAX_VERTICES * sizeof(Vertex)};

    // Create the vertex buffer
    vk::BufferUsageFlags constexpr vertexbufferUsage {vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst};
    vk::MemoryPropertyFlags constexpr vertexBufferMemoryProperties {vk::MemoryPropertyFlagBits::eDeviceLocal};
    core.createBuffer(
        bufferSize,
        vertexbufferUsage,
        vertexBufferMemoryProperties,
        buffer,
        memory
    );
}