#pragma once

#include "icore.hpp"
#include "buffer.hpp"
#include "vertex.hpp"

class VertexBuffer {
public:
    vk::raii::Buffer buffer {nullptr};
    vk::raii::DeviceMemory memory {nullptr};
    size_t const MAX_VERTICES {4000};
    
    void create(ICore const & core);
};