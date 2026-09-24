#pragma once

#include "icore.hpp"
#include "buffer.hpp"
#include "bufferManager.hpp"

template <typename T>
class ManagedBuffer {
public:
    vk::raii::Buffer buffer {nullptr};
    vk::raii::DeviceMemory memory {nullptr};

    void init(
        ICore const & core,
        size_t const maxItems,
        vk::BufferUsageFlags const bufferUsage,
        vk::MemoryPropertyFlags const bufferMemoryProperties
    );
    void pushItems(
        ICore const & core,
        vk::raii::CommandPool const & commandPool,
        std::vector<T> const & items
    );
    uint32_t getOffset(size_t index) const;

private:
    BufferManager<T> manager {nullptr};
};

template <typename T>
void ManagedBuffer<T>::init(
    ICore const & core,
    size_t const maxItems,
    vk::BufferUsageFlags const bufferUsage,
    vk::MemoryPropertyFlags const bufferMemoryProperties
) {
    // Create the buffer
    vk::DeviceSize const bufferSize {maxItems * sizeof(T)};

    Buffer::create(
        core.getPhysicalDevice(),
        core.getDevice(),
        bufferSize,
        bufferUsage,
        bufferMemoryProperties,
        buffer,
        memory
    );

    // Create the buffer manager
    manager = BufferManager<T>(buffer);
}

template <typename T>
void ManagedBuffer<T>::pushItems(
    ICore const & core,
    vk::raii::CommandPool const & commandPool,
    std::vector<T> const & items
) {
    manager.pushItems(core.getPhysicalDevice(), core.getDevice(), core.getQueue(), commandPool, items);
}

template <typename T>
uint32_t ManagedBuffer<T>::getOffset(size_t index) const {
    return manager.getOffset(index);
}