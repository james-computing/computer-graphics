#pragma once

#include <iostream>
#include <vector>
#include "vertex.hpp"

#include "../libraries/stb/stb_image.h"

class Model {
private:
    std::string const modelPath {"./models/viking_room.obj"};
    std::string const texturePath {"./textures/viking_room.png"};

    void loadTexture();
    void loadVertices();
public:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    stbi_uc * pixels;
    int textureWidth;
    int textureHeight;
    int textureChannels;

    ~Model();
    void load();
};