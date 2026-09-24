#pragma once

#include "icore.hpp"
#include "transform.hpp"

// Multiple instances for a model.
// Use a single class, because will create a single SSBO (Shader Storage Buffer Object),
// instead of an array of uniform buffers or something else.

class ModelsInstances {
private:
    std::vector<uint32_t> instanceCounts; // instance count for each loaded model
    size_t instanceCountTotal;
    size_t numModels;
    std::vector<Transform> transforms; // transforms for each instance for each model
    std::vector<glm::mat4> modelMatrices;

public:
    std::vector<vk::raii::Buffer> shaderStorageBuffers; // store the model matrices here, a buffer for frame in flight
    
private:
    std::vector<vk::raii::DeviceMemory> shaderStorageBuffersMemories;
    std::vector<void*> shaderStorageBuffersMapped; // pointers to transfer data from host to shader storage buffers

    void createSSBOs(ICore const & core, uint32_t const maxFramesInFlight, size_t const instanceCountTotal);
    void updateTransforms(float const deltaTime);
    void computeModelMatrices(std::vector<glm::mat4> & modelMatrices);

public:
    void init(
        ICore const & core,
        uint32_t const maxFramesInFlight,
        size_t const numModels,
        std::vector<uint32_t> const & instanceCounts
    );
    void updateShaderStorageBuffer(uint32_t const frameIndex, float const deltaTime);
    uint32_t getInstanceCountTotal() const;
    uint32_t getInstanceCount(size_t const index) const;
};