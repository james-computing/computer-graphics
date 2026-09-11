#include "../include/modelInstances.hpp"

#include "../include/buffer.hpp"
#include "../include/ubos.hpp"
#include <iostream>

void ModelInstances::init(ICore const & core, uint32_t const maxFramesInFlight) {
    std::cout << "ModelInstances::init" << std::endl;
    createSSBOs(core, maxFramesInFlight);

    transforms.reserve(instanceCount);
    float constexpr s {0.5f};
    float constexpr space {1.0f};
    float x = 0.0f;
    float z = 0.0f;
    for (uint32_t instance {0}; instance < instanceCount; ++instance) {
        std::cout << "(x,z) = (" << x << "," << z << ")" << std::endl;

        Transform & transform {transforms[instance]};
        transform.location = glm::vec3(x, 0.0f, z);
        transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        transform.scale = glm::vec3(s, s, s);
        
        x += space;
        if (x > 2 * space) {
            x = 0.0f;
            z += space;
        }
    }
};

void ModelInstances::createSSBOs(ICore const & core, uint32_t const maxFramesInFlight) {
    for (size_t i {0}; i < maxFramesInFlight; ++i) {
        // Create shader storage buffer, allocate memory for it and bind it
        uint32_t constexpr MAX_INSTANCE_COUNT {16};
        vk::DeviceSize constexpr bufferSize {MAX_INSTANCE_COUNT * sizeof(glm::mat4)};
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

glm::mat4 ModelInstances::getModelMatrix(uint32_t const instance) const {
    return transforms[instance].getModelMatrix();
};

void ModelInstances::updateTransform(uint32_t const instance, float const deltaTime) {
    float constexpr rotationSpeed {1.0};
    transforms[instance].rotation.y += rotationSpeed * deltaTime;
}

void ModelInstances::updateShaderStorageBuffer(uint32_t const frameIndex, float const deltaTime) {
    std::vector<glm::mat4> modelMatrices;
    modelMatrices.reserve(instanceCount);

    for (uint32_t instance {0}; instance < instanceCount; ++instance) {
        updateTransform(instance, deltaTime);
        modelMatrices[instance] = getModelMatrix(instance);
    }

    // Copy the model matrices to the corresponding uniform buffer memory.
    size_t const size {instanceCount * sizeof(glm::mat4)};
    memcpy(shaderStorageBuffersMapped[frameIndex], modelMatrices.data(), size);
}

uint32_t ModelInstances::getInstanceCount() const {
    return instanceCount;
}
