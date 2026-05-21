#pragma once

#include "icore.hpp"
#include "buffer.hpp"

class IndexBuffer {
public:
    vk::raii::Buffer buffer {nullptr};
    vk::raii::DeviceMemory memory {nullptr};
    size_t const MAX_INDICES {12000};
    uint32_t numIndices {0};

    void create(ICore const & core);
};