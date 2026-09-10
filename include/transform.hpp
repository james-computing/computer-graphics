#pragma once

#include <glm/glm.hpp> // for vectors and matrices for computer graphics

struct Transform {
    glm::vec3 location;
    glm::vec3 rotation;
    glm::vec3 scale;

    glm::mat4 getModelMatrix() const;
};