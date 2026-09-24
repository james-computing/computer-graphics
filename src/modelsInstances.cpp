#include "../include/modelsInstances.hpp"

#include "../include/buffer.hpp"
#include "../include/ubos.hpp"
#include <iostream>
#include <numeric> // for accumulate

void ModelsInstances::init(
    ICore const & core,
    uint32_t const maxFramesInFlight,
    size_t const numModels,
    std::vector<uint32_t> const & instanceCounts
) {
    std::cout << "ModelInstances::init" << std::endl;
    if (instanceCounts.size() != numModels) {
        throw std::runtime_error("instanceCounts vector should have size numModels!");
    }
    this->instanceCounts = instanceCounts;
    instanceCountTotal = std::accumulate(instanceCounts.begin(), instanceCounts.end(), 0);
    this->numModels = numModels;

    createSSBOs(core, maxFramesInFlight, instanceCountTotal);

    transforms.reserve(instanceCountTotal);
    float constexpr s {0.5f};
    float constexpr space {1.0f};
    float x = 0.0f;
    float z = 0.0f;
    size_t i {0};
    for (size_t modelIndex {0}; modelIndex < numModels; ++modelIndex) {
        uint32_t const instanceCount {instanceCounts[modelIndex]};
        for (uint32_t instance {0}; instance < instanceCount; ++instance) {
            std::cout << "(x,z) = (" << x << "," << z << ")" << std::endl;

            Transform & transform {transforms[i]};
            transform.location = glm::vec3(x, 0.0f, z);
            transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
            transform.scale = glm::vec3(s, s, s);
            
            x += space;
            if (x > 2 * space) {
                x = 0.0f;
                z += space;
            }
            ++i;
        }
    }

    modelMatrices.reserve(instanceCountTotal);
};

void ModelsInstances::createSSBOs(ICore const & core, uint32_t const maxFramesInFlight, size_t const instanceCountTotal) {
    for (size_t i {0}; i < maxFramesInFlight; ++i) {
        // Create shader storage buffer, allocate memory for it and bind it
        //uint32_t constexpr MAX_INSTANCE_TOTAL {16};
        vk::DeviceSize const bufferSize {instanceCountTotal * sizeof(glm::mat4)};
        vk::BufferUsageFlags constexpr bufferUsage {vk::BufferUsageFlagBits::eStorageBuffer};
        // host visible = can be used to transfer from CPU to GPU
        // host coherent = doesn't need flush or invalidate the memory manually
        vk::MemoryPropertyFlags constexpr memoryProperties {
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        };
        vk::raii::Buffer buffer {nullptr};
        vk::raii::DeviceMemory bufferMemory {nullptr};
        Buffer::create(
            core.getPhysicalDevice(),
            core.getDevice(),
            bufferSize,
            bufferUsage,
            memoryProperties,
            buffer,
            bufferMemory
        );
        shaderStorageBuffers.emplace_back(std::move(buffer));
        shaderStorageBuffersMemories.emplace_back(std::move(bufferMemory));
        
        // Map buffer to a pointer, so we can transfer data from the pointer to the buffer
        shaderStorageBuffersMapped.emplace_back(shaderStorageBuffersMemories[i].mapMemory(0, bufferSize));
    }
}

void ModelsInstances::updateTransforms(float const deltaTime) {
    float constexpr rotationSpeed {1.0};
    for (size_t i {0}; i < instanceCountTotal; ++i) {
        transforms[i].rotation.y += rotationSpeed * deltaTime;
    }
}

void ModelsInstances::computeModelMatrices(std::vector<glm::mat4> & modelMatrices) {
    for (size_t i {0}; i < instanceCountTotal; ++i) {
        modelMatrices[i] = transforms[i].getModelMatrix();
    }
}

void ModelsInstances::updateShaderStorageBuffer(uint32_t const frameIndex, float const deltaTime) {
    updateTransforms(deltaTime);
    computeModelMatrices(modelMatrices);

    // Copy the model matrices to the corresponding uniform buffer memory.
    size_t const size {instanceCountTotal * sizeof(glm::mat4)};
    memcpy(shaderStorageBuffersMapped[frameIndex], modelMatrices.data(), size);
}

uint32_t ModelsInstances::getInstanceCountTotal() const {
    return instanceCountTotal;
}


uint32_t ModelsInstances::getInstanceCount(size_t const index) const {
    return instanceCounts[index];
}