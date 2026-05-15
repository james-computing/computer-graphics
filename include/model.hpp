#pragma once

#include <iostream>
#include <vector>
#include "vertex.hpp"

#include "../libraries/stb/stb_image.h"
#include "core.hpp"

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