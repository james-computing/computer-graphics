#include "../include/texture.hpp"

void Texture::computeMipLevels(int textureWidth, int textureHeight) {
    // sum 1 for the original image to have a mip level
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(textureWidth, textureHeight)))) + 1;
}

void Texture::createTextureImageView(
    Core const & core,
    vk::raii::Image & textureImage,
    vk::raii::ImageView & textureImageView,
    uint32_t mipLevels
) {
    textureImageView = core.createImageView(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, mipLevels);
}

void Texture::load(Core & core, char const * texturePath) {
    std::cout << "Loading texture" << std::endl;
    // texturePath.c_str()
    stbi_uc * pixels = stbi_load(texturePath, &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("Failed to load texture.");
    }
    std::cout << "textureWidth = " << textureWidth
    << ", textureHeight = " << textureHeight
    << ", textureChannels = " << textureChannels
    << std::endl;

    computeMipLevels(textureWidth, textureHeight);

    // copy pixels data to texture image
    // texture resources
    core.createTextureImage(textureWidth, textureHeight, pixels, textureImage, textureImageMemory, mipLevels);

    // cleanup
    stbi_image_free(pixels);

    // depends on textureImage and mipLevels
    createTextureImageView(core, textureImage, textureImageView, mipLevels);
}