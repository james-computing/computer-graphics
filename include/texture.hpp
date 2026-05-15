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
    vk::raii::Image image {nullptr};
    vk::raii::DeviceMemory imageMemory {nullptr};
    vk::raii::ImageView imageView {nullptr};

    // METHODS //
    void computeMipLevels(int const textureWidth, int const textureHeight);
    void transitionTextureImageLayout(
        Core const & core,
        vk::ImageLayout const oldLayout,
        vk::ImageLayout const newLayout
    ) const;
    void generateMipmaps(Core const & core, vk::Format imageFormat) const;
    void createTextureImage(Core const & core, stbi_uc const * const pixels);
    void createTextureImageView(Core const & core);
    void load(Core const & core, char const * const texturePath);
};