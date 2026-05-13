#include "../include/model.hpp"

// Include here to avoid multiple implementation.
// STB is for loading the texture image.
#define STB_IMAGE_IMPLEMENTATION
#include "../libraries/stb/stb_image.h"

// Tiny obj loader is for loading the 3d model.
#define TINYOBJLOADER_IMPLEMENTATION
#include "../libraries/tinyobjloader/tiny_obj_loader.h"

Model::~Model() {
    // cleanup
    stbi_image_free(pixels);
}

void Model::loadTexture() {
    std::cout << "Loading texture" << std::endl;
    pixels = stbi_load(texturePath.c_str(), &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("Failed to load texture.");
    }
    std::cout << "textureWidth = " << textureWidth
    << ", textureHeight = " << textureHeight
    << ", textureChannels = " << textureChannels
    << std::endl;
}

void Model::loadVertices() {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
        std::cerr << "Failed to load model" << std::endl;
        throw std::runtime_error(warn + err);
    }

    size_t triple_vertex_index;
    size_t double_texture_index;
    // Make a map to store a vertex and the index attribute to it in its first appearance
    std::unordered_map<Vertex, uint32_t> uniqueVertices {};
    uint32_t newVertexIndex;
    for (tinyobj::shape_t const & shape : shapes) {
        for (auto const & index : shape.mesh.indices) {
            Vertex vertex;

            triple_vertex_index = 3 * index.vertex_index;
            vertex.position = {
                attrib.vertices[triple_vertex_index],
                attrib.vertices[triple_vertex_index + 1],
                attrib.vertices[triple_vertex_index + 2]
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
}

void Model::load() {
    loadTexture();
    loadVertices();
}