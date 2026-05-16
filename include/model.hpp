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
    uint32_t numIndices;
public:
    Texture texture;

    // METHODS //
private:
    void loadVertices(Core const & core, std::string_view const modelPath);

public:
    void load(Core const & core, std::string_view const modelPath, std::string_view const texturePath);
    uint32_t getNumIndices() const;
};