#pragma once

// An object will use a model, which has the texture and vertex data.
// It will also have additional information to modify its position, rotation and scale.

#include "model.hpp"
#include "mvp.hpp"
#include <glm/glm.hpp> // for vectors and matrices for computer graphics
#include <glm/gtc/matrix_transform.hpp> // for model view projection
#include <chrono> // for model view projection

class Object {
public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    std::vector<vk::raii::DescriptorSet> descriptorSets;

private:
    Model const * _modelPtr;

    std::vector<vk::raii::Buffer> uniformBuffers; // model view projection matrices are stored in uniform buffers
    std::vector<vk::raii::DeviceMemory> uniformBuffersMemories;
    std::vector<void*> uniformBuffersMapped; // pointers to transfer data from host to uniform buffers
    
public:
    void init(Model const & model, ICore const & core, IRenderer const & renderer, vk::raii::Sampler const & textureSampler);

    void updateUniformBuffer(uint32_t const frameIndex, uint32_t const swapChainExtentWidth, uint32_t const swapChainExtentHeight);

private:
    glm::mat4 getModelMatrix() const;
    void createUniformBuffers(ICore const & core, uint32_t const quantity);
    void updateDescriptorSets(ICore const & core, vk::raii::Sampler const & textureSampler, uint32_t const quantity) const;
};