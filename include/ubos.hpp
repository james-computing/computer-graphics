#pragma once

#include <glm/glm.hpp> // for vectors and matrices for computer graphics

struct ObjectUBO {
    alignas(16) glm::mat4 model;
};

struct CameraUBO {
    alignas(16) glm::mat4 pv; // projection * view
};