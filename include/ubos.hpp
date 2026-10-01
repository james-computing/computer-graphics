#pragma once

#include <glm/glm.hpp> // for vectors and matrices for computer graphics

struct alignas(16) CameraUBO {
    glm::mat4 pv; // projection * view
    glm::vec3 location; // for reflection with ray queries. Must align entire struct to put a vec3 at the end.
};

static_assert(sizeof(CameraUBO) % 16 == 0);