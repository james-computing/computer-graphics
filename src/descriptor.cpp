#include "../include/descriptor.hpp"

void Descriptor::createDescriptorSetLayout(vk::raii::Device const & device) {
    vk::DescriptorSetLayoutBinding constexpr uboDescriptorSetLayoutBinding {
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = nullptr
    };

    vk::DescriptorSetLayoutBinding constexpr combinedImageSamplerDescriptorSetLayoutBinding {
        .binding = 1,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eFragment,
        .pImmutableSamplers = nullptr
    };

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings {
        uboDescriptorSetLayoutBinding, 
        combinedImageSamplerDescriptorSetLayoutBinding
    };

    vk::DescriptorSetLayoutCreateInfo const descriptorSetLayoutCreateInfo {
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };

    setLayout = vk::raii::DescriptorSetLayout(device, descriptorSetLayoutCreateInfo);
}

void Descriptor::createDescriptorPool(
    vk::raii::Device const & device,
    uint32_t const uniformBufferCount,
    uint32_t const combinedImageSamplerCount,
    uint32_t const maxSets
) {
    vk::DescriptorPoolSize const uniformBufferDescriptorPoolSize {
        .type = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = uniformBufferCount
    };

    vk::DescriptorPoolSize const combinedImageSamplerDescriptorPoolSize {
        .type = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = combinedImageSamplerCount
    };

    std::array<vk::DescriptorPoolSize, 2> descriptorPoolSizes {
        uniformBufferDescriptorPoolSize,
        combinedImageSamplerDescriptorPoolSize
    };

    vk::DescriptorPoolCreateInfo const descriptorPoolCreateInfo {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = maxSets,
        .poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
        .pPoolSizes = descriptorPoolSizes.data()
    };

    pool = vk::raii::DescriptorPool(device, descriptorPoolCreateInfo);
}

void Descriptor::create(
    vk::raii::Device const & device,
    uint32_t const uniformBufferCount,
    uint32_t const combinedImageSamplerCount,
    uint32_t const maxSets
) {
    createDescriptorSetLayout(device);
    createDescriptorPool(device, uniformBufferCount, combinedImageSamplerCount, maxSets);
}