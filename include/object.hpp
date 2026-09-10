#pragma once

// An object will use a model, which has the texture and vertex data.
// It will also have additional information to modify its position, rotation and scale.
/*
#include "irenderer.hpp"
#include "buffer.hpp"
#include "ubos.hpp"
#include <chrono> // for animation

#include "transforms.hpp"

class Object {
public:
    glm::vec3 location;
    glm::vec3 rotation;
    glm::vec3 scale;

    std::vector<vk::raii::Buffer> uniformBuffers; // model matrix is stored in uniform buffers

private:
    std::vector<vk::raii::DeviceMemory> uniformBuffersMemories;
    std::vector<void*> uniformBuffersMapped; // pointers to transfer data from host to uniform buffers
    
public:
    void init(ICore const & core, uint32_t const maxFramesInFlight);

    void updateUniformBuffer(uint32_t const frameIndex);

private:
    glm::mat4 getModelMatrix() const;
    void createUniformBuffers(ICore const & core, uint32_t const maxFramesInFlight);
    void updateModelMatrix();
};
*/