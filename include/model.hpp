#pragma once

#include <iostream>
#include <vector>

#include "../libraries/stb/stb_image.h"

#include "vertex.hpp"
#include "core.hpp"
#include "texture.hpp"

class Model {
private:
    std::string const modelPath {"./models/viking_room.obj"};
    std::string const texturePath {"./textures/viking_room.png"};

    vk::raii::Image textureImage {nullptr};
    vk::raii::DeviceMemory textureImageMemory {nullptr};

    void loadVertices(Core & core);
    void loadTexture(Core & core);
public:
    vk::raii::ImageView textureImageView {nullptr};
    int textureWidth;
    int textureHeight;
    int textureChannels;
    uint32_t mipLevels;

    // Data to access vertex data in vertex and index buffers
    size_t vertexBufferLocation;
    size_t numVertices;
    size_t indexBufferLocation;
    size_t numIndices;

    void load(Core & core);
};