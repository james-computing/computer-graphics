#include "include/indirectDraw.hpp"

#include "include/buffer.hpp"

#include <iostream>

// Later replace indexCount and instanceCount by model data
void IndirectDraw::createIndirectCommands(
    ICore const & core,
    size_t const numModels,
    ModelData const & modelData,
    ModelsInstances const & modelsInstances
) {
    // Create draw indexed indirect commands
    // One drawIndexedIndirectCommand for each model
    uint32_t instanceOffset {0};
    uint32_t instanceCount;
    for (size_t i {0}; i < numModels; ++i) {
        instanceCount = modelsInstances.getInstanceCount(i);
        vk::DrawIndexedIndirectCommand drawIndexedIndirectCommand {
            .indexCount = modelData.indexCounts[i], // number of indices of the model
            .instanceCount = instanceCount, // number of instances for the model
            .firstIndex = modelData.indexBuffer.getOffset(i),
            .vertexOffset = static_cast<int32_t>(modelData.vertexBuffer.getOffset(i)),
            .firstInstance = instanceOffset
        };

        drawIndexedIndirectCommands.push_back(drawIndexedIndirectCommand);
        instanceOffset += instanceCount;
    }
}

void IndirectDraw::createBuffers(ICore const & core, uint32_t const maxFramesInFlight, size_t const numModels) {
    vk::DeviceSize const indirectBufferSize {numModels * sizeof(vk::DrawIndexedIndirectCommand)};
    vk::BufferUsageFlags constexpr indirectBufferUsageFlags {vk::BufferUsageFlagBits::eIndirectBuffer};
    vk::MemoryPropertyFlags constexpr memoryProperties {
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent |
        vk::MemoryPropertyFlagBits::eDeviceLocal
    };

    // Create indirect buffers and respective memories
    for (size_t i {0}; i < maxFramesInFlight; ++i) {
        vk::raii::Buffer buffer {nullptr};
        vk::raii::DeviceMemory bufferMemory {nullptr};
        Buffer::create(
            core.getPhysicalDevice(),
            core.getDevice(),
            indirectBufferSize,
            indirectBufferUsageFlags,
            memoryProperties,
            buffer,
            bufferMemory
        );
        buffers.emplace_back(std::move(buffer));
        buffersMemories.emplace_back(std::move(bufferMemory));

        // Copy draw indirect command to indirect buffer
        // map buffer memory to data pointer
        void * data {buffersMemories[i].mapMemory(0, indirectBufferSize)};
        
        // copy drawIndexedIndirectCommands to data, which copies to the buffer memory
        memcpy(data, drawIndexedIndirectCommands.data(), indirectBufferSize);
        buffersMemories[i].unmapMemory();
        data = nullptr;
    }
}

void IndirectDraw::create(
    ICore const & core,
    uint32_t const maxFramesInFlight,
    size_t const numModels,
    ModelData const & modelData,
    ModelsInstances const & modelsInstances
) {
    createIndirectCommands(core, numModels, modelData, modelsInstances);
    createBuffers(core, maxFramesInFlight, numModels);
}