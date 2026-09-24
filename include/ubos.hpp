#pragma once

#include <glm/glm.hpp> // for vectors and matrices for computer graphics

struct CameraUBO {
    alignas(16) glm::mat4 pv; // projection * view
};