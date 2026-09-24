#pragma once

#include "managedBuffer.hpp"
#include "vertex.hpp"
#include "texture.hpp"

class ModelData {
public:
    ManagedBuffer<Vertex> vertexBuffer;
    ManagedBuffer<uint32_t> indexBuffer;
    std::vector<uint32_t> indexCounts;
    // use unique pointer, because Texture doesn't have a copy constructor, because of vk::raii::Image.
    std::vector<Texture> textures;

    void init(ICore const & core, uint32_t numTextures);

    void load(
        ICore const & core,
        vk::raii::CommandPool const & commandPool,
        std::string_view const modelPath,
        std::string_view const texturePath
    );

private:
    void loadVertices(ICore const & core, vk::raii::CommandPool const & commandPool, std::string_view const modelPath);
};