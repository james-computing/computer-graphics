#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <cmath>

#include "../libraries/stb/stb_image.h"
#include "../include/core.hpp"

class Texture {
private:
    vk::raii::Image textureImage {nullptr};
    vk::raii::DeviceMemory textureImageMemory {nullptr};
    vk::raii::ImageView textureImageView {nullptr};
    vk::raii::Sampler textureSampler {nullptr};

    uint32_t mipLevels;

    // METHODS

    void createTextureImage(Core & core, int textureWidth, int textureHeight, stbi_uc * pixels);

    void transitionTextureImageLayout(
        Core & core,
        vk::raii::Image const & image,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        uint32_t mipLevels
    ) const;

    void createTextureImageView(Core & core);
    void createTextureSampler(Core & core);

    void generateMipmaps(
        Core & core,
        vk::raii::Image & image,
        vk::Format imageFormat,
        int32_t textureWidth,
        int32_t textureHeight,
        uint32_t mipLevels
    );

public:
    void init(Core & core, int textureWidth, int textureHeight, stbi_uc * pixels);
    vk::raii::ImageView & getImageView();
    vk::raii::Sampler & getSampler();
};