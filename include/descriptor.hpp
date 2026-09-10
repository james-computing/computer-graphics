#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

/*
enum Binding {
    combinedImageSampler,
    cameraUBO,
    objectUBO
};
*/

class Descriptor {
private:
    // The descriptor pool is only used when allocating descriptor sets
    vk::raii::DescriptorPool pool {nullptr};

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
    //vk::raii::DescriptorSetLayout setLayoutObject {nullptr};
    vk::raii::DescriptorSetLayout setLayoutModelInstances {nullptr};

    // The declaration order determines the destruction order.
    // For this reason, the descriptor sets must be declared after the descriptor pool.

    // Couldn't use separate vectors, because can't break the vector obtained from device.allocateDescriptorSets
    /*
    std::vector<vk::raii::DescriptorSet> setsCombinedImageSampler;
    std::vector<vk::raii::DescriptorSet> setsCamera;
    std::vector<vk::raii::DescriptorSet> setsObject;
    */
    // Use a single vector instead
    std::vector<vk::raii::DescriptorSet> sets;

    void create(
        vk::raii::Device const & device,
        uint32_t const maxFramesInFlights
    );

    void updateDescriptorSets(
        vk::raii::Device const & device,
        uint32_t const maxFramesInFlight,
        vk::raii::Sampler const & textureSampler,
        vk::raii::ImageView const & textureImageView,
        std::vector<vk::raii::Buffer> const & cameraUniformBuffers,
        //std::vector<vk::raii::Buffer> const & objectUniformBuffers
        std::vector<vk::raii::Buffer> const & modelInstancesSSBOs,
        uint32_t const instanceCount
    ) const;
};