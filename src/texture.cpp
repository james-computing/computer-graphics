#include "../include/texture.hpp"

namespace Texture {

uint32_t mipLevels(int textureWidth, int textureHeight) {
    // sum 1 for the original image to have a mip level
    return static_cast<uint32_t>(std::floor(std::log2(std::max(textureWidth, textureHeight)))) + 1;
}

}