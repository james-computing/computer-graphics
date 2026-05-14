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
    // Both vertices and indices are only used to load the data from files.
    // After that, the data is copied to the vertex and index buffers and these
    // variables are never used again.
    // It makes more sense than to just automate the process of loading vertex data into
    // the vertex buffer. A solution would be to keep track of where the data will be
    // stored in the buffers and get rid of these variables.
    // The same goes for the pixels. We just use them to create the texture image and image view.
    // After that, the pixels variable is useless and we should get rid of it.
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    stbi_uc * pixels;

    int textureWidth;
    int textureHeight;
    int textureChannels;

    // Data to access vertex data in vertex and index buffers
    size_t vertexBufferLocation;
    size_t numVertices;
    size_t indexBufferLocation;
    size_t numIndices;

    ~Model();
    void load();
};