#pragma once

#include "icore.hpp"
#include "buffer.hpp"
#include <iostream>

/*
    A simple generic buffer manager. It can only push new items, keeping track of the offsets for each vector pushed.
    Unlike a stack, there is no pop operation, or any operation to clean the buffer in some way.

    The buffer stores information as
    | items0 | items1 | ...
    The offsets are the indices for the beggining of each items vector inside the buffer. Therefore,
    items0 has offset 0, items1 has offset items0.size() and etc.
*/

template <typename T>
class BufferManager {
private:
    vk::raii::Buffer const * bufferPtr {nullptr};
    std::vector<uint32_t> offsets;
    uint32_t nextOffset {0};

public:
    BufferManager(vk::raii::Buffer const & buffer);
    ~BufferManager();
    uint32_t getOffset(size_t const index) const;
    void pushItems(
        vk::raii::PhysicalDevice const & physicalDevice,
        vk::raii::Device const & device,
        vk::raii::Queue const & queue,
        vk::raii::CommandPool const & commandPool,
        std::vector<T> const & items
    );
};

template <typename T>
BufferManager<T>::BufferManager(vk::raii::Buffer const & buffer) {
    bufferPtr = &buffer;
}

template <typename T>
uint32_t BufferManager<T>::getOffset(size_t const index) const {
    return offsets[index];
}

template <typename T>
void BufferManager<T>::pushItems(
    vk::raii::PhysicalDevice const & physicalDevice,
    vk::raii::Device const & device,
    vk::raii::Queue const & queue,
    vk::raii::CommandPool const & commandPool,
    std::vector<T> const & items
) {
    // push the items to the buffer
    std::cout << "Buffer::copyVectorToBuffer" << std::endl;
    std::cout << "items.size() = " << items.size() << std::endl;
    std::cout << "offset = " << nextOffset << std::endl;
    Buffer::copyVectorToBuffer<T>(
        physicalDevice,
        device,
        queue,
        commandPool,
        items,
        nextOffset, // nextOffset is now the offset for the current push
        *bufferPtr
    );

    std::cout << "offsets.emplace_back" << std::endl;
    // update offsets
    offsets.emplace_back(nextOffset); // nextOffset is now the offset for the current push

    // update nextOffset, for the next push
    nextOffset += static_cast<uint32_t>(items.size());
}

template <typename T>
BufferManager<T>::~BufferManager() {
    bufferPtr = nullptr;
}