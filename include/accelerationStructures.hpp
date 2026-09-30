#pragma once

#include "modelData.hpp"
#include "modelsInstances.hpp"

// Acceleration structures for ray tracing
class AccelerationStructures {
public:
    // BLAS
    std::vector<vk::raii::Buffer> blasBuffers;
    std::vector<vk::raii::DeviceMemory> blasBufferMemories;
    std::vector<vk::raii::AccelerationStructureKHR> blasHandles;

    // INSTANCE
    std::vector<vk::AccelerationStructureInstanceKHR> instances;
    vk::raii::Buffer instanceBuffer {nullptr};
    vk::raii::DeviceMemory instanceBufferMemory {nullptr};

    // TLAS
    vk::raii::Buffer tlasBuffer {nullptr};
    vk::raii::DeviceMemory tlasBufferMemory {nullptr};
    vk::raii::Buffer tlasScratchBuffer {nullptr};
    vk::raii::DeviceMemory tlasScratchBufferMemory {nullptr};
    vk::raii::AccelerationStructureKHR tlas {nullptr};

    void create(
        vk::raii::PhysicalDevice const & physicalDevice,
        vk::raii::Device const & device,
        vk::raii::Queue const & queue,
        vk::raii::CommandPool const & commandPool,
        ModelData const & modelData,
        uint32_t const numModels,
        ModelsInstances const & modelsInstances
    );

    void createBLAS(
        vk::raii::PhysicalDevice const & physicalDevice,
        vk::raii::Device const & device,
        vk::raii::Queue const & queue,
        vk::raii::CommandPool const & commandPool,
        ModelData const & modelData,
        uint32_t const numModels
    );

    void createInstances(
        vk::raii::Device const & device,
        ModelsInstances const & modelsInstances,
        uint32_t const numModels
    );

    void createTLAS(
        vk::raii::PhysicalDevice const & physicalDevice,
        vk::raii::Device const & device,
        vk::raii::Queue const & queue,
        vk::raii::CommandPool const & commandPool
    );

    void updateTLAS(
        vk::raii::PhysicalDevice const & physicalDevice,
        vk::raii::Device const & device,
        vk::raii::Queue const & queue,
        vk::raii::CommandPool const & commandPool,
        std::vector<glm::mat4> const & modelMatrices
    );
};