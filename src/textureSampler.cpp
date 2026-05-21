#include "../include/textureSampler.hpp"

namespace TextureSampler {

void create(
    vk::raii::PhysicalDevice const & physicalDevice,
    vk::raii::Device const & device,
    vk::raii::Sampler & textureSampler
) {
    vk::PhysicalDeviceProperties physicalDeviceProperties {physicalDevice.getProperties()};

    vk::SamplerCreateInfo const samplerCreateInfo {
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .mipLodBias = 0.0f,
        .anisotropyEnable = vk::True,
        .maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy,
        .compareEnable = vk::False,
        .compareOp = vk::CompareOp::eAlways,
        .minLod = 0.0f,
        .maxLod = vk::LodClampNone,
        .borderColor = vk::BorderColor::eIntOpaqueBlack,
        .unnormalizedCoordinates = vk::False
    };

    textureSampler = vk::raii::Sampler(device, samplerCreateInfo);
}

}