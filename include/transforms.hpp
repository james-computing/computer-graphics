#pragma once

// Need glm in the header, because of the return types.
// Force depth in [0,1], for correct perspective matrix for Vulkan
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // for glm::translate

namespace transforms {

glm::mat4 perspective_general_opengl(float const l, float const r, float const b, float const t, float const n, float const f);
glm::mat4 perspective_opengl(float const field_of_view, float const aspect_ration, float const n, float const f);

glm::mat4 perspective_general_vulkan(float const l, float const r, float const b, float const t, float const n, float const f);
glm::mat4 perspective_vulkan(float const field_of_view, float const aspect_ration, float const n, float const f);
//glm::mat4 perspective_general_vulkan_wrong(float const l, float const r, float const b, float const t, float const n, float const f);
glm::mat4 orthographic_vulkan(float const l, float const r, float const b, float const t, float const n, float const f);

glm::mat4 lookAtRotation(glm::vec3 const eye, glm::vec3 const center, glm::vec3 const up);
glm::mat4 lookAt(glm::vec3 const eye, glm::vec3 const center, glm::vec3 const up);

}