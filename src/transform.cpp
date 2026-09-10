#include "../include/transform.hpp"

// Force depth in [0,1], for correct perspective matrix for Vulkan
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp> // for vectors and matrices for computer graphics
#include <glm/gtc/matrix_transform.hpp> // for model view projection
#include <glm/gtx/string_cast.hpp>

glm::mat4 Transform::getModelMatrix() const {
    // start with identity matrix
    glm::mat4 model {glm::mat4(1.0f)};

    model = glm::translate(model, location);

    model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0, 0));
    model = glm::rotate(model, rotation.y, glm::vec3(0, 1.0f, 0));
    model = glm::rotate(model, rotation.z, glm::vec3(0, 0, 1.0f));

    model = glm::scale(model, scale);

    return model;
}