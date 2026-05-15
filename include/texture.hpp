#pragma once

#include <cmath>
#include <cstdint> // For uint32_t
#include "core.hpp"

class Texture {
    // VARIABLES
private:
    int textureWidth;
    int textureHeight;
    int textureChannels;
    uint32_t mipLevels;
public:
    vk::raii::Image textureImage {nullptr};
    vk::raii::DeviceMemory textureImageMemory {nullptr};
    vk::raii::ImageView textureImageView {nullptr};

    // METHODS //
    void computeMipLevels(int textureWidth, int textureHeight);

    void createTextureImageView(
        Core const & core,
        vk::raii::Image & textureImage,
        vk::raii::ImageView & textureImageView,
        uint32_t mipLevels
    );

    void load(Core & core, char const * texturePath);
};