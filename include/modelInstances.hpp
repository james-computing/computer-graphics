#pragma once

#include "icore.hpp"
#include "transform.hpp"

// Multiple instances for a model.
// Use a single class, because will create a single SSBO (Shader Storage Buffer Object),
// instead of an array of uniform buffers or something else.

class ModelInstances {
private:
    uint32_t const instanceCount {1};
    std::vector<Transform> transforms;

public:
    std::vector<vk::raii::Buffer> shaderStorageBuffers; // store the model matrices here, a buffer for frame in flight
    
private:
    std::vector<vk::raii::DeviceMemory> shaderStorageBuffersMemories;
    std::vector<void*> shaderStorageBuffersMapped; // pointers to transfer data from host to shader storage buffers

    void createSSBOs(ICore const & core, uint32_t const maxFramesInFlight);
    glm::mat4 getModelMatrix(uint32_t const instance) const;
    void updateTransform(uint32_t const instance, float const deltaTime);

public:
    void init(ICore const & core, uint32_t const maxFramesInFlight);
    void updateShaderStorageBuffer(uint32_t const frameIndex, float const deltaTime);
    uint32_t getInstanceCount() const;
};