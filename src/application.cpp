#include "../include/application.hpp"

void Application::init() {
    std::cout << "Application init" << std::endl;
    core.init();

    std::cout << "Create texture sampler" << std::endl;
    // depends on the logical and physical devices.
    // Used in createDescriptorSets.
    core.createTextureSampler(textureSampler);
    
    std::cout << "model load" << std::endl;
    model.load(core, modelPath, texturePath);

    // depends on MAX_FRAMES_IN_FLIGHT. Also calls createBuffer, which depends on the logical device.
    createUniformBuffers();

    // depends on descriptorSetLayout, descriptorPool, uniform buffer, texture sampler, texture image view...
    createDescriptorSets();
}

void Application::run() {
    init();
    
    while (core.shouldContinue()) {
        core.pollEvents();
        updateUniformBuffer(core.getFrameIndex());
        core.drawFrame(descriptorSets);
    }

    core.cleanup();
}

void Application::createUniformBuffers() {
    uint32_t const MAX_FRAMES_IN_FLIGHT {core.getMaxFramesInFlight()};
    for (size_t i {0}; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        // Create uniform buffer, allocate memory for it and bind it
        vk::DeviceSize constexpr bufferSize {sizeof(UniformBufferObject)};
        vk::BufferUsageFlags constexpr bufferUsage {vk::BufferUsageFlagBits::eUniformBuffer};
        vk::MemoryPropertyFlags constexpr memoryProperties {
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        };
        vk::raii::Buffer buffer {nullptr};
        vk::raii::DeviceMemory bufferMemory {nullptr};
        core.createBuffer(
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

void Application::updateUniformBuffer(uint32_t frameIndex) {
    // Get the start time from the first call to this function.
    // Later calls won't update the start time.
    static auto const startTime {std::chrono::high_resolution_clock::now()};

    // Compute the time elapsed from start time to now. Elapsed time will parameterize the rotation.
    auto const currenTime {std::chrono::high_resolution_clock::now()};
    float const elapsedTime {std::chrono::duration<float, std::chrono::seconds::period>(currenTime - startTime).count()};

    // Update the uniform buffer
    UniformBufferObject ubo;

    glm::vec3 constexpr up {glm::vec3(0.0f, 0.0f, 1.0f)};

    // Rotate model around the z axis, according to the elapsed time.
    glm::mat4 constexpr identity {glm::mat4(1.0f)};
    ubo.model = glm::rotate(identity, elapsedTime * glm::radians(90.0f), up);

    // View the model from a 45° angle
    glm::vec3 constexpr eye {glm::vec3(2.0f, 2.0f, 2.0f)};
    glm::vec3 constexpr center {glm::vec3(0.0f, 0.0f, 0.0f)};
    ubo.view = glm::lookAt(eye, center, up);

    // Perspective projection
    float const aspectRatio {static_cast<float>(core.getSwapChainExtentWidth())/static_cast<float>(core.getSwapChainExtentHeight())};
    float constexpr near {0.1f};
    float constexpr far {10.f};
    ubo.proj = glm::perspective(glm::radians(45.0f), aspectRatio, near, far);
    // GLM was made for OpenGL. For Vulkan we need to flip the sign of the Y scaling factor.
    ubo.proj[1][1] *= -1;

    // Copy the ubo to the corresponding uniform buffer memory.
    // It would be more efficient to use push constants.
    memcpy(uniformBuffersMapped[frameIndex], &ubo, sizeof(ubo));
}

void Application::allocateDescriptorSets() {
    core.allocateDescriptorSets(core.getMaxFramesInFlight(), descriptorSets);
}

void Application::updateDescriptorSets() const {
    // Configure descriptor sets.
    // Maybe could build an array of vk::WriteDescriptorSet and call device.updateDescriptorSets once.
    uint32_t const MAX_FRAMES_IN_FLIGHT {core.getMaxFramesInFlight()};
    for (size_t i {0}; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        // uniform buffer

        vk::DescriptorBufferInfo const descriptorBufferInfo {
            .buffer = uniformBuffers[i],
            .offset = 0,
            .range = sizeof(UniformBufferObject)
        };

        vk::WriteDescriptorSet const uniformBufferWriteDescriptorSet {
            .dstSet = descriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pBufferInfo = &descriptorBufferInfo
        };

        // Combined image sampler

        vk::DescriptorImageInfo const descriptorImageInfo {
            .sampler = textureSampler,
            .imageView = model.texture.imageView,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };

        vk::WriteDescriptorSet const combinedImageSamplerWriteDescriptorSet {
            .dstSet = descriptorSets[i],
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = &descriptorImageInfo
        };

        std::vector<vk::WriteDescriptorSet> writeDescriptorSets {
            uniformBufferWriteDescriptorSet,
            combinedImageSamplerWriteDescriptorSet
        };

        // update
        core.updateDescriptorSets(writeDescriptorSets);
    }
}

void Application::createDescriptorSets() {
    allocateDescriptorSets();
    updateDescriptorSets();
}