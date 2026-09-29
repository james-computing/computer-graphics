#include "../include/modelData.hpp"

// Include here to avoid multiple implementation.
// STB is for loading the texture image.
#define STB_IMAGE_IMPLEMENTATION
#include "../libraries/stb/stb_image.h"

// Tiny obj loader is for loading the 3d model.
#define TINYOBJLOADER_IMPLEMENTATION
#include "../libraries/tinyobjloader/tiny_obj_loader.h" // already inlcudes <cstring>, which has memcpy

void ModelData::init(ICore const & core, uint32_t const numTextures) {
    // Create the vertex buffer
    std::cout << "Create vertex buffer" << std::endl;
    size_t const maxVertices {numTextures * 3566}; // exactly for viking model
    vk::BufferUsageFlags constexpr vertexbufferUsage {
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst |
        vk::BufferUsageFlagBits::eShaderDeviceAddress | // for acceleration structures
        vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
    };
    vk::MemoryPropertyFlags constexpr vertexBufferMemoryProperties {vk::MemoryPropertyFlagBits::eDeviceLocal};
    std::cout << "vertexBuffer.init" << std::endl;
    vertexBuffer.init(core, maxVertices, vertexbufferUsage, vertexBufferMemoryProperties);

    // Create the index buffer
    std::cout << "Create index buffer" << std::endl;
    size_t const maxIndices {numTextures * 11484}; // exactly for viking model
    vk::BufferUsageFlags constexpr indexbufferUsage {
        vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eTransferDst |
        vk::BufferUsageFlagBits::eShaderDeviceAddress | // for acceleration structures
        vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR // for acceleration structures
        //vk::BufferUsageFlagBits::eStorageBuffer // for acceleration structures?
    };
    vk::MemoryPropertyFlags constexpr indexBufferMemoryProperties {vk::MemoryPropertyFlagBits::eDeviceLocal};
    indexBuffer.init(core, maxIndices, indexbufferUsage, indexBufferMemoryProperties);
}

void ModelData::loadVertices(ICore const & core, vk::raii::CommandPool const & commandPool, std::string_view const modelPath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.data())) {
        std::cerr << "Failed to load model" << std::endl;
        throw std::runtime_error(warn + err);
    }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    size_t triple_vertex_index;
    size_t double_texture_index;
    // Make a map to store a vertex and the index attribute to it in its first appearance
    std::unordered_map<Vertex, uint32_t> uniqueVertices {};
    uint32_t newVertexIndex;
    size_t numShapes {0};
    for (tinyobj::shape_t const & shape : shapes) {
        ++numShapes;
        for (auto const & index : shape.mesh.indices) {
            Vertex vertex;

            triple_vertex_index = 3 * index.vertex_index;
            vertex.position = {
                /*
                attrib.vertices[triple_vertex_index],
                attrib.vertices[triple_vertex_index + 1],
                attrib.vertices[triple_vertex_index + 2]
                */
                // Correct for y up
                attrib.vertices[triple_vertex_index + 1],
                attrib.vertices[triple_vertex_index + 2],
                attrib.vertices[triple_vertex_index]
            };

            double_texture_index = 2 * index.texcoord_index;
            vertex.textureCoord = {
                attrib.texcoords[double_texture_index],
                1.0f - attrib.texcoords[double_texture_index + 1]
            };

            // Do we need a color?
            //vertex.color = {1.0f, 1.0f, 1.0f};

            // If the vertex is new, store it in uniqueVertices and give it an index
            if (uniqueVertices.count(vertex) == 0) {
                // Create an index
                newVertexIndex = static_cast<uint32_t>(vertices.size());
                // Store the vertex and its index in the map
                uniqueVertices[vertex] = newVertexIndex;
                // Store the vertex is the vertices vector
                vertices.emplace_back(vertex);
            }
            
            // Store the index of the vector
            indices.emplace_back(uniqueVertices[vertex]);
        }
    }
    std::cout << "number of shapes = " << numShapes << std::endl;

    size_t const vertexCount {vertices.size()};
    size_t const indexCount {indices.size()};
    std::cout << "number of vertices = " << vertexCount << std::endl;
    std::cout << "number of indices = " << indexCount << std::endl;

    vertexBuffer.pushItems(core, commandPool, vertices);
    indexBuffer.pushItems(core, commandPool, indices);
    indexCounts.emplace_back(indexCount);
    vertexCounts.emplace_back(vertexCount);
}

void ModelData::load(
    ICore const & core,
    vk::raii::CommandPool const & commandPool,
    std::string_view const modelPath,
    std::string_view const texturePath
) {
    Texture texture;
    texture.load(core, commandPool, texturePath.data());
    textures.emplace_back(std::move(texture));

    loadVertices(core, commandPool, modelPath.data());
}