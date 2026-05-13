#pragma once

#include <iostream>
#include <vector>
#include "vertex.hpp"

class Model {
private:
    std::string const modelPath {"./models/viking_room.obj"};
public:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    // texture

    void load();
};