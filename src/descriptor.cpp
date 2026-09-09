#include "../include/descriptor.hpp"
#include "../include/ubos.hpp"

#include <iostream>

void Descriptor::createDescriptorSetLayouts(vk::raii::Device const & device) {
    // combined image sampler
    vk::DescriptorSetLayoutBinding constexpr descriptorSetLayoutBindingCombinedImageSampler {
        .binding = 0,//Binding::combinedImageSampler,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eFragment,
        .pImmutableSamplers = nullptr
    };

    vk::DescriptorSetLayoutBinding bindingsCombinedImageSampler[] {descriptorSetLayoutBindingCombinedImageSampler};
    vk::DescriptorSetLayoutCreateInfo const descriptorSetLayoutCreateInfoCombinedImageSampler {
        .bindingCount = 1,
        .pBindings = bindingsCombinedImageSampler
    };

    setLayoutCombinedImageSampler = vk::raii::DescriptorSetLayout(device, descriptorSetLayoutCreateInfoCombinedImageSampler);

    // camera ubo
    vk::DescriptorSetLayoutBinding constexpr descriptorSetLayoutBindingCameraUBO {
        .binding = 0,//Binding::cameraUBO,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = nullptr
    };

    vk::DescriptorSetLayoutBinding bindingsCamera[] {descriptorSetLayoutBindingCameraUBO};
    vk::DescriptorSetLayoutCreateInfo const descriptorSetLayoutCreateInfoCamera {
        .bindingCount = 1,
        .pBindings = bindingsCamera
    };

    setLayoutCamera = vk::raii::DescriptorSetLayout(device, descriptorSetLayoutCreateInfoCamera);

    // object ubo
    vk::DescriptorSetLayoutBinding constexpr descriptorSetLayoutBindingObjectUBO {
        .binding = 0,//Binding::objectUBO,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = nullptr
    };

    vk::DescriptorSetLayoutBinding bindingsObject[] {descriptorSetLayoutBindingObjectUBO};
    vk::DescriptorSetLayoutCreateInfo const descriptorSetLayoutCreateInfoObject {
        .bindingCount = 1,
        .pBindings = bindingsObject
    };

    setLayoutObject = vk::raii::DescriptorSetLayout(device, descriptorSetLayoutCreateInfoObject);

    /*
    std::array<vk::DescriptorSetLayoutBinding, 3> bindings {
        cameraUBODescriptorSetLayoutBinding,
        objectUBODescriptorSetLayoutBinding, 
        combinedImageSamplerDescriptorSetLayoutBinding
    };

    vk::DescriptorSetLayoutCreateInfo const descriptorSetLayoutCreateInfo {
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };

    setLayout = vk::raii::DescriptorSetLayout(device, descriptorSetLayoutCreateInfo);
    */
}

void Descriptor::createDescriptorPool(
    vk::raii::Device const & device,
    uint32_t const maxFramesInFlight
) {
    uint32_t const uniformBufferCount {2 * maxFramesInFlight}; // camera and object
    uint32_t const combinedImageSamplerCount {maxFramesInFlight};
    uint32_t const maxSets {uniformBufferCount + combinedImageSamplerCount}; // sum, because each thing is in a separate descriptor set

    vk::DescriptorPoolSize const combinedImageSamplerDescriptorPoolSize {
        .type = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = combinedImageSamplerCount
    };

    vk::DescriptorPoolSize const uniformBufferDescriptorPoolSize {
        .type = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = uniformBufferCount
    };

    std::array<vk::DescriptorPoolSize, 2> descriptorPoolSizes {
        combinedImageSamplerDescriptorPoolSize,
        uniformBufferDescriptorPoolSize
    };

    vk::DescriptorPoolCreateInfo const descriptorPoolCreateInfo {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = maxSets,
        .poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
        .pPoolSizes = descriptorPoolSizes.data()
    };

    pool = vk::raii::DescriptorPool(device, descriptorPoolCreateInfo);
}

void Descriptor::allocateDescriptorSets(
    vk::raii::Device const & device,
    uint32_t const maxFramesInFlight
) {
    uint32_t const descriptorSetCount {3 * maxFramesInFlight}; // all descriptor sets

    // Vector with descriptorSetCount copies of *descriptorSetLayout.
    // It is needed because descriptorSetAllocateInfo receives an array of layouts.
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts; //{std::vector(descriptorSetCount, *descriptorSetLayout)};
    descriptorSetLayouts.reserve(descriptorSetCount);
    descriptorSetLayouts.insert(descriptorSetLayouts.end(), maxFramesInFlight, *setLayoutCombinedImageSampler);
    descriptorSetLayouts.insert(descriptorSetLayouts.end(), maxFramesInFlight, *setLayoutCamera);
    descriptorSetLayouts.insert(descriptorSetLayouts.end(), maxFramesInFlight, *setLayoutObject);
    

    // Allocate descriptor sets
    vk::DescriptorSetAllocateInfo const descriptorSetAllocateInfo {
        .descriptorPool = pool,
        .descriptorSetCount = descriptorSetCount,
        .pSetLayouts = descriptorSetLayouts.data()
    };

    sets = device.allocateDescriptorSets(descriptorSetAllocateInfo);
    
    /*
    // error: use of deleted function ‘vk::raii::DescriptorSet::DescriptorSet()’
    // Distribute descriptorSets to the respective vectors
    setsCombinedImageSampler.resize(maxFramesInFlight);
    setsCamera.resize(maxFramesInFlight);
    setsObject.resize(maxFramesInFlight);
    
    size_t i {0};
    for (; i < maxFramesInFlight; ++i) {
        setsCombinedImageSampler.emplace_back(std::move(descriptorSets[i]));
    }
    for (; i < 2 * maxFramesInFlight; ++i) {
        setsCamera.emplace_back(std::move(descriptorSets[i]));
    }
    for (; i < 3 * maxFramesInFlight; ++i) {
        setsObject.emplace_back(std::move(descriptorSets[i]));
    }
    */
}

void Descriptor::updateDescriptorSets(
    vk::raii::Device const & device,
    uint32_t const maxFramesInFlight,
    vk::raii::Sampler const & textureSampler,
    vk::raii::ImageView const & textureImageView,
    std::vector<vk::raii::Buffer> const & cameraUniformBuffers,
    std::vector<vk::raii::Buffer> const & objectUniformBuffers
) const {
    std::cout << "Descriptor::updateDescriptorSets" << std::endl;
    // Configure descriptor sets.
    // It is one write for each descriptor set.
    size_t i {0};
    for (; i < maxFramesInFlight; ++i) {
        // Combined image sampler
        vk::DescriptorImageInfo const descriptorImageInfo {
            .sampler = *textureSampler,
            .imageView = *textureImageView,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };

        vk::WriteDescriptorSet const writeDescriptorSetCombinedImageSampler {
            .dstSet = *sets[i],
            .dstBinding = 0,//Binding::combinedImageSampler,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = &descriptorImageInfo,
        };

        std::vector<vk::WriteDescriptorSet> const writeDescriptorSets {
            writeDescriptorSetCombinedImageSampler
        };
        device.updateDescriptorSets(writeDescriptorSets, {});
    }

    size_t j {0};
    for (; i < 2 * maxFramesInFlight; ++i) {
        // camera uniform buffer
        vk::DescriptorBufferInfo const descriptorBufferInfoCamera {
            .buffer = *(cameraUniformBuffers[j]),
            .offset = 0,
            .range = sizeof(CameraUBO)
        };

        vk::WriteDescriptorSet const writeDescriptorSetCameraUniformBuffer {
            .dstSet = *sets[i],
            .dstBinding = 0,//Binding::cameraUBO,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pBufferInfo = &descriptorBufferInfoCamera
        };

        std::vector<vk::WriteDescriptorSet> const writeDescriptorSets {
            writeDescriptorSetCameraUniformBuffer
        };
        device.updateDescriptorSets(writeDescriptorSets, {});
        ++j;
    }

    size_t k {0};
    for (; i < 3 * maxFramesInFlight; ++i) {
        // object uniform buffer
        vk::DescriptorBufferInfo const descriptorBufferInfoObject {
            .buffer = *objectUniformBuffers[k],
            .offset = 0,
            .range = sizeof(ObjectUBO)
        };

        vk::WriteDescriptorSet const writeDescriptorSetObjectUniformBuffer {
            .dstSet = *sets[i],
            .dstBinding = 0,//Binding::objectUBO,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pBufferInfo = &descriptorBufferInfoObject
        };

        // update
        /*
        std::vector<vk::WriteDescriptorSet> writeDescriptorSets {
            writeDescriptorSetCombinedImageSampler,
            writeDescriptorSetCameraUniformBuffer,
            writeDescriptorSetObjectUniformBuffer
        };
        */

        std::vector<vk::WriteDescriptorSet> const writeDescriptorSets {
            writeDescriptorSetObjectUniformBuffer
        };
        device.updateDescriptorSets(writeDescriptorSets, {});
        ++k;
    }
}

void Descriptor::create(
    vk::raii::Device const & device,
    uint32_t const maxFramesInFlights
) {
    std::cout << "Create descriptor set layouts" << std::endl;
    createDescriptorSetLayouts(device);
    std::cout << "Create descriptor pool" << std::endl;
    createDescriptorPool(device, maxFramesInFlights);
    std::cout << "Allocate descriptor sets" << std::endl;
    allocateDescriptorSets(device, maxFramesInFlights);
}