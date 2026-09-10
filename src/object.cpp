#include "../include/object.hpp"
/*
#include <cmath>
#include <cstring> // for memcpy
#include <iostream>

// Force depth in [0,1], for correct perspective matrix for Vulkan
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp> // for vectors and matrices for computer graphics
#include <glm/gtc/matrix_transform.hpp> // for model view projection
#include <glm/gtx/string_cast.hpp>

void Object::init(ICore const & core, uint32_t const maxFramesInFlight) {
    createUniformBuffers(core, maxFramesInFlight);

    location = glm::vec3(0, 0, 0);
    rotation = glm::vec3(0, 0, 0);
    scale = glm::vec3(1.0f, 1.0f, 1.0f);
}

void Object::createUniformBuffers(ICore const & core, uint32_t const maxFramesInFlight) {
    for (size_t i {0}; i < maxFramesInFlight; ++i) {
        // Create uniform buffer, allocate memory for it and bind it
        vk::DeviceSize constexpr bufferSize {sizeof(ObjectUBO)};
        vk::BufferUsageFlags constexpr bufferUsage {vk::BufferUsageFlagBits::eUniformBuffer};
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
        uniformBuffers.emplace_back(std::move(buffer));
        uniformBuffersMemories.emplace_back(std::move(bufferMemory));
        
        // Map uniform buffer to a pointer, so we can transfer data from the pointer to the uniform buffer
        uniformBuffersMapped.emplace_back(uniformBuffersMemories[i].mapMemory(0, bufferSize));
    }
}

glm::mat4 Object::getModelMatrix() const {
    // start with identity matrix
    glm::mat4 model {glm::mat4(1.0f)};

    model = glm::translate(model, location);

    model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0, 0));
    model = glm::rotate(model, rotation.y, glm::vec3(0, 1.0f, 0));
    model = glm::rotate(model, rotation.z, glm::vec3(0, 0, 1.0f));

    model = glm::scale(model, scale);

    return model;
};

void Object::updateModelMatrix() {
    // Get the start time from the first call to this function.
    // Later calls won't update the start time.
    static auto const startTime {std::chrono::high_resolution_clock::now()};

    // Compute the time elapsed from start time to now. Elapsed time will parameterize the rotation.
    auto const currentTime {std::chrono::high_resolution_clock::now()};
    float const elapsedTime {std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count()};

    // y is up in Vulkan
    rotation.y = elapsedTime * glm::radians(90.0f);
}

void Object::updateUniformBuffer(uint32_t const frameIndex) {
    updateModelMatrix();

    // Update the uniform buffer
    ObjectUBO ubo;

    ubo.model = getModelMatrix();

    // Copy the ubo to the corresponding uniform buffer memory.
    memcpy(uniformBuffersMapped[frameIndex], &ubo, sizeof(ubo));
}
*/