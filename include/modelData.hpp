#pragma once

#include "managedBuffer.hpp"
#include "vertex.hpp"
#include "texture.hpp"

class ModelData {
public:
    // Use managed buffer where there is multiple data for each model. Otherwise, just use a buffer.
    ManagedBuffer<Vertex> vertexBuffer;
    std::vector<uint32_t> vertexCounts; // for BLAS (bottom level acceleration structure) in ray tracing
    ManagedBuffer<uint32_t> indexBuffer;
    std::vector<uint32_t> indexCounts;
    // use unique pointer, because Texture doesn't have a copy constructor, because of vk::raii::Image.
    std::vector<Texture> textures;

    // For ray query
    std::vector<bool> alphaCuts;
    vk::raii::Buffer indexOffsetsBuffer {nullptr};
    vk::raii::DeviceMemory indexOffsetsBufferMemory {nullptr};
    //vk::raii::Buffer uvBuffer {nullptr};
    //vk::raii::DeviceMemory uvBufferMemory {nullptr};

    void init(ICore const & core, uint32_t numTextures);

    void load(
        ICore const & core,
        vk::raii::CommandPool const & commandPool,
        std::string_view const modelPath,
        std::string_view const texturePath,
        bool const alphaCut
    );

    void updateIndexOffsetsBuffer(ICore const & core, vk::raii::CommandPool const & commandPool);

private:
    void loadVertices(ICore const & core, vk::raii::CommandPool const & commandPool, std::string_view const modelPath);
};