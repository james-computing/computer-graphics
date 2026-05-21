#pragma once

#include <cmath>
#include <cstdint> // For uint32_t
#include <iostream>
#include "irenderer.hpp"
#include "../include/singleTimeCommands.hpp"
#include "../include/buffer.hpp"
#include "../libraries/stb/stb_image.h"

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
        ICore const & core,
        IRenderer const & renderer,
        vk::ImageLayout const oldLayout,
        vk::ImageLayout const newLayout
    ) const;
    void generateMipmaps(ICore const & core, IRenderer const & renderer, vk::Format imageFormat) const;
    void createTextureImage(ICore const & core, IRenderer const & renderer, stbi_uc const * const pixels);
    void createTextureImageView(ICore const & core);
    void load(ICore const & core, IRenderer const & renderer, char const * const texturePath);
};