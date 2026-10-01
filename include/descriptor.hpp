#pragma once

#include "texture.hpp"
#include "modelData.hpp"

class Descriptor {
private:
    // The descriptor pool is only used when allocating descriptor sets
    vk::raii::DescriptorPool pool {nullptr};

    uint32_t const maxTextures {16};

    void createDescriptorSetLayouts(vk::raii::Device const & device);
    void createDescriptorPool(
        vk::raii::Device const & device,
        uint32_t const maxFramesInFlight
    );
    void allocateDescriptorSets(
        vk::raii::Device const & device,
        uint32_t const maxFramesInFlight
    );

public:
    // The descriptor set layout is used by the graphics pipeline, to know how to bind the descriptor sets
    vk::raii::DescriptorSetLayout setLayoutCombinedImageSampler {nullptr};
    vk::raii::DescriptorSetLayout setLayoutCamera {nullptr};
    vk::raii::DescriptorSetLayout setLayoutModelsInstances {nullptr};
    vk::raii::DescriptorSetLayout setLayoutAccelerationStructures {nullptr};
    vk::raii::DescriptorSetLayout setLayoutIndexBuffer {nullptr};
    vk::raii::DescriptorSetLayout setLayoutIndexOffsetsBuffer {nullptr};
    vk::raii::DescriptorSetLayout setLayoutVertexBuffer {nullptr};

    // The declaration order determines the destruction order.
    // For this reason, the descriptor sets must be declared after the descriptor pool.

    std::vector<vk::raii::DescriptorSet> setsCombinedImageSampler;
    std::vector<vk::raii::DescriptorSet> setsCamera;
    std::vector<vk::raii::DescriptorSet> setsModelsInstances;
    std::vector<vk::raii::DescriptorSet> setsAccelerationStructures;
    std::vector<vk::raii::DescriptorSet> setsIndexBuffer;
    std::vector<vk::raii::DescriptorSet> setsIndexOffsetsBuffer;
    std::vector<vk::raii::DescriptorSet> setsVertexBuffer;

    void create(
        vk::raii::Device const & device,
        uint32_t const maxFramesInFlights
    );

    void updateDescriptorSets(
        vk::raii::Device const & device,
        uint32_t const maxFramesInFlight,
        vk::raii::Sampler const & textureSampler,
        std::vector<Texture> const & textures,
        std::vector<vk::raii::Buffer> const & cameraUniformBuffers,
        std::vector<vk::raii::Buffer> const & modelsInstancesSSBOs,
        uint32_t const instanceCountTotal,
        vk::raii::AccelerationStructureKHR const & tlas,
        ModelData const & modelData,
        uint32_t const numModels
    ) const;
};