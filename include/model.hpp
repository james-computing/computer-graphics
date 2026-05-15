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
    // Data to access vertex data in vertex and index buffers
    size_t vertexBufferLocation;
    size_t numVertices;
    size_t indexBufferLocation;
    size_t numIndices;
public:
    Texture texture;

private:
    // METHODS //
    void loadVertices(Core & core, std::string_view modelPath);

public:
    void load(Core & core, std::string_view modelPath, std::string_view texturePath);
};