#pragma once

#include <iostream>
#include <vector>

#include "../libraries/stb/stb_image.h"

#include "vertex.hpp"
#include "core.hpp"
#include "texture.hpp"

class Model {
    // VARIABLES //
private:
    std::string const modelPath {"./models/viking_room.obj"};
    std::string const texturePath {"./textures/viking_room.png"};

    // Data to access vertex data in vertex and index buffers
    size_t vertexBufferLocation;
    size_t numVertices;
    size_t indexBufferLocation;
    size_t numIndices;
public:
    Texture texture;

private:
    // METHODS //
    void loadVertices(Core & core);

public:
    void load(Core & core);
};