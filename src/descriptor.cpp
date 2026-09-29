#include "../include/descriptor.hpp"
#include "../include/ubos.hpp"

#include <iostream>

void Descriptor::createDescriptorSetLayouts(vk::raii::Device const & device) {
    // combined image sampler
    vk::DescriptorSetLayoutBinding const descriptorSetLayoutBindingCombinedImageSampler {
        .binding = 0,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = maxTextures,
        .stageFlags = vk::ShaderStageFlagBits::eFragment,
        .pImmutableSamplers = nullptr
    };

    vk::DescriptorBindingFlags constexpr descriptorBindingFlagsCombinedImageSampler[] {
        vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eVariableDescriptorCount
    };
    vk::DescriptorSetLayoutBindingFlagsCreateInfo const descriptorSetLayoutBindingFlagsCreateInfoCombinedImageSampler {
        .bindingCount = 1, // number of elements in the array pBindingFlags
        .pBindingFlags = descriptorBindingFlagsCombinedImageSampler,
    };

    vk::DescriptorSetLayoutBinding const bindingsCombinedImageSampler[] {descriptorSetLayoutBindingCombinedImageSampler};
    vk::DescriptorSetLayoutCreateInfo const descriptorSetLayoutCreateInfoCombinedImageSampler {
        .pNext = &descriptorSetLayoutBindingFlagsCreateInfoCombinedImageSampler, // chain the flags
        .bindingCount = 1,
        .pBindings = bindingsCombinedImageSampler
    };

    setLayoutCombinedImageSampler = vk::raii::DescriptorSetLayout(device, descriptorSetLayoutCreateInfoCombinedImageSampler);

    // camera ubo
    vk::DescriptorSetLayoutBinding constexpr descriptorSetLayoutBindingCameraUBO {
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = nullptr
    };

    vk::DescriptorSetLayoutBinding const bindingsCamera[] {descriptorSetLayoutBindingCameraUBO};
    vk::DescriptorSetLayoutCreateInfo const descriptorSetLayoutCreateInfoCamera {
        .bindingCount = 1,
        .pBindings = bindingsCamera
    };

    setLayoutCamera = vk::raii::DescriptorSetLayout(device, descriptorSetLayoutCreateInfoCamera);

    // object ubo
    /*
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
    */

    vk::DescriptorSetLayoutBinding constexpr descriptorSetLayoutBindingModelsInstances {
        .binding = 0,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = nullptr
    };

    vk::DescriptorSetLayoutBinding const bindingsModelsInstances[] {descriptorSetLayoutBindingModelsInstances};
    vk::DescriptorSetLayoutCreateInfo const descriptorSetLayoutCreateInfoModelsInstances {
        .bindingCount = 1,
        .pBindings = bindingsModelsInstances
    };

    setLayoutModelsInstances = vk::raii::DescriptorSetLayout(device, descriptorSetLayoutCreateInfoModelsInstances);

    // Acceleration structures
    vk::DescriptorSetLayoutBinding constexpr descriptorSetLayoutBindingAccelerationStructures {
        .binding = 0,
        .descriptorType = vk::DescriptorType::eAccelerationStructureKHR,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eFragment,
        .pImmutableSamplers = nullptr
    };

    vk::DescriptorSetLayoutBinding const bindingsAccelerationStructures[] {descriptorSetLayoutBindingAccelerationStructures};
    vk::DescriptorSetLayoutCreateInfo const descriptorSetLayoutCreateInfoAccelerationStructures {
        .bindingCount = 1,
        .pBindings = bindingsAccelerationStructures
    };

    setLayoutAccelerationStructures = vk::raii::DescriptorSetLayout(device, descriptorSetLayoutCreateInfoAccelerationStructures);
}

void Descriptor::createDescriptorPool(
    vk::raii::Device const & device,
    uint32_t const maxFramesInFlight
) {
    uint32_t const uniformBufferCount {maxFramesInFlight}; // only camera
    uint32_t const combinedImageSamplerCount {maxTextures * maxFramesInFlight};
    uint32_t const shaderStorageBufferCount {maxFramesInFlight};
    uint32_t const accelerationStructuresCount {maxFramesInFlight}; // a TLAS for frame
    uint32_t const maxSets {
        // sum, because each thing is in a separate descriptor set
        uniformBufferCount + combinedImageSamplerCount + shaderStorageBufferCount + accelerationStructuresCount
    };

    vk::DescriptorPoolSize const descriptorPoolSizeCombinedImageSampler {
        .type = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = combinedImageSamplerCount
    };

    vk::DescriptorPoolSize const descriptorPoolSizeUniformBuffer {
        .type = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = uniformBufferCount
    };

    vk::DescriptorPoolSize const descriptorPoolSizeShaderStorageBuffer {
        .type = vk::DescriptorType::eStorageBuffer,
        .descriptorCount = shaderStorageBufferCount
    };

    vk::DescriptorPoolSize const descriptorPoolSizeAccelerationStructures {
        .type = vk::DescriptorType::eAccelerationStructureKHR,
        .descriptorCount = accelerationStructuresCount
    };

    std::array<vk::DescriptorPoolSize, 4> descriptorPoolSizes {
        descriptorPoolSizeCombinedImageSampler,
        descriptorPoolSizeUniformBuffer,
        descriptorPoolSizeShaderStorageBuffer,
        descriptorPoolSizeAccelerationStructures
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
    // Vector with descriptorSetCount copies of *descriptorSetLayout.
    // It is needed because descriptorSetAllocateInfo receives an array of layouts.
    //std::vector<vk::DescriptorSetLayout> descriptorSetLayouts; //{std::vector(descriptorSetCount, *descriptorSetLayout)};
    std::vector<vk::DescriptorSetLayout> const descriptorSetLayoutsCombinedImageSampler {
        std::vector<vk::DescriptorSetLayout>(maxFramesInFlight, *setLayoutCombinedImageSampler)
    };
    std::vector<vk::DescriptorSetLayout> const descriptorSetLayoutsCamera {
        std::vector<vk::DescriptorSetLayout>(maxFramesInFlight, *setLayoutCamera)
    };
    std::vector<vk::DescriptorSetLayout> const descriptorSetLayoutsModelsInstances {
        std::vector<vk::DescriptorSetLayout>(maxFramesInFlight, *setLayoutModelsInstances)
    };
    std::vector<vk::DescriptorSetLayout> const descriptorSetLayoutsAccelerationStructures {
        std::vector<vk::DescriptorSetLayout>(maxFramesInFlight, *setLayoutAccelerationStructures)
    };
    
    std::vector<uint32_t> const descriptorCountsCombinedImageSampler {std::vector<uint32_t>(maxFramesInFlight, maxTextures)};
    vk::DescriptorSetVariableDescriptorCountAllocateInfo const variableDescriptorCountInfo {
        .descriptorSetCount = maxFramesInFlight, // number of combined image sampler descriptor sets
        .pDescriptorCounts = descriptorCountsCombinedImageSampler.data()
    };

    // Allocate descriptor sets
    vk::DescriptorSetAllocateInfo const descriptorSetAllocateInfoCombinedImageSampler {
        .pNext = &variableDescriptorCountInfo,
        .descriptorPool = pool,
        .descriptorSetCount = maxFramesInFlight,
        .pSetLayouts = descriptorSetLayoutsCombinedImageSampler.data()
    };
    setsCombinedImageSampler = device.allocateDescriptorSets(descriptorSetAllocateInfoCombinedImageSampler);

    vk::DescriptorSetAllocateInfo const descriptorSetAllocateInfoCamera {
        .descriptorPool = pool,
        .descriptorSetCount = maxFramesInFlight,
        .pSetLayouts = descriptorSetLayoutsCamera.data()
    };
    setsCamera = device.allocateDescriptorSets(descriptorSetAllocateInfoCamera);

    vk::DescriptorSetAllocateInfo const descriptorSetAllocateInfoModelsInstances {
        .descriptorPool = pool,
        .descriptorSetCount = maxFramesInFlight,
        .pSetLayouts = descriptorSetLayoutsModelsInstances.data()
    };
    setsModelsInstances = device.allocateDescriptorSets(descriptorSetAllocateInfoModelsInstances);

    vk::DescriptorSetAllocateInfo const descriptorSetAllocateInfoAccelerationStructures {
        .descriptorPool = pool,
        .descriptorSetCount = maxFramesInFlight,
        .pSetLayouts = descriptorSetLayoutsAccelerationStructures.data()
    };
    setsAccelerationStructures = device.allocateDescriptorSets(descriptorSetAllocateInfoAccelerationStructures);
    
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
    std::vector<Texture> const & textures,
    std::vector<vk::raii::Buffer> const & cameraUniformBuffers,
    std::vector<vk::raii::Buffer> const & modelsInstancesSSBOs,
    uint32_t const instanceCountTotal,
    vk::raii::AccelerationStructureKHR const & tlas
) const {
    std::cout << "Descriptor::updateDescriptorSets" << std::endl;
    // Configure descriptor sets.
    // It is one write for each descriptor set.

    // Combined image sampler
    size_t const numTextures {textures.size()};
    std::vector<vk::DescriptorImageInfo> descriptorImageInfos;
    descriptorImageInfos.reserve(numTextures);
    for(size_t i {0}; i < numTextures; ++i) {
        vk::DescriptorImageInfo const descriptorImageInfo {
            .sampler = *textureSampler,
            .imageView = *textures[i].imageView,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };
        descriptorImageInfos.emplace_back(descriptorImageInfo);
    }

    for (size_t i {0}; i < maxFramesInFlight; ++i) {
        vk::WriteDescriptorSet const writeDescriptorSetCombinedImageSampler {
            .dstSet = *setsCombinedImageSampler[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = static_cast<uint32_t>(numTextures), // length of pImageInfo array, since the descriptorType is eCombinedImageSampler
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = descriptorImageInfos.data(),
        };

        std::vector<vk::WriteDescriptorSet> const writeDescriptorSets {
            writeDescriptorSetCombinedImageSampler
        };
        device.updateDescriptorSets(writeDescriptorSets, {});
    }

    // camera uniform buffer
    for (size_t i {0}; i < maxFramesInFlight; ++i) {
        vk::DescriptorBufferInfo const descriptorBufferInfoCamera {
            .buffer = *(cameraUniformBuffers[i]),
            .offset = 0,
            .range = sizeof(CameraUBO)
        };

        vk::WriteDescriptorSet const writeDescriptorSetCameraUniformBuffer {
            .dstSet = *setsCamera[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pBufferInfo = &descriptorBufferInfoCamera
        };

        std::vector<vk::WriteDescriptorSet> const writeDescriptorSets {
            writeDescriptorSetCameraUniformBuffer
        };
        device.updateDescriptorSets(writeDescriptorSets, {});
    }

    // Models instances (before was objects)
    for (size_t i {0}; i < maxFramesInFlight; ++i) {
        // object uniform buffer
        /*
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

        std::vector<vk::WriteDescriptorSet> const writeDescriptorSets {
            writeDescriptorSetObjectUniformBuffer
        };
        device.updateDescriptorSets(writeDescriptorSets, {});
        */

        vk::DescriptorBufferInfo const descriptorBufferInfoModelsInstances {
            .buffer = *modelsInstancesSSBOs[i],
            .offset = 0,
            .range =  instanceCountTotal * sizeof(glm::mat4)
        };

        vk::WriteDescriptorSet const writeDescriptorSetModelsInstances {
            .dstSet = *setsModelsInstances[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &descriptorBufferInfoModelsInstances
        };

        std::vector<vk::WriteDescriptorSet> const writeDescriptorSets {
            writeDescriptorSetModelsInstances
        };
        device.updateDescriptorSets(writeDescriptorSets, {});
    }

    for (size_t i {0}; i < maxFramesInFlight; ++i) {
        /*
        vk::DescriptorBufferInfo const descriptorBufferInfoAccelerationStructures {
            .buffer = 
        };*/
        vk::WriteDescriptorSetAccelerationStructureKHR const writeDescriptorSetAccelerationStructure {
            .accelerationStructureCount = 1,
            .pAccelerationStructures = &*tlas
        };

        vk::WriteDescriptorSet const writeDescriptorSet {
            .pNext = &writeDescriptorSetAccelerationStructure,
            .dstSet = *setsAccelerationStructures[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eAccelerationStructureKHR
        };

        std::vector<vk::WriteDescriptorSet> const writeDescriptorSets {
            writeDescriptorSet
        };
        device.updateDescriptorSets(writeDescriptorSets, {});
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