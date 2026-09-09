#include "../include/transforms.hpp"

#include <iostream>
#include <cmath>

// glm is column major, so entries of a matrix M are accessed as M[column][row]

namespace transforms {

glm::mat4 perspective_general_opengl(float const l, float const r, float const b, float const t, float const n, float const f) {
    glm::mat4 projection = glm::mat4(0.0f);

    projection[0][0] = 2*n/(r-l);
    projection[2][0] = (r+l)/(r-l);
    projection[1][1] = 2*n/(t-b);
    projection[2][1] = (t+b)/(t-b);
    projection[2][2] = -(f+n)/(f-n);
    projection[3][2] = -(2*f*n)/(f-n);
    projection[2][3] = -1;

    return projection;
}

// field of view is vertical
// aspect ration = width/height
glm::mat4 perspective_opengl(float const field_of_view, float const aspect_ration, float const n, float const f) {
    glm::mat4 projection = glm::mat4(0.0f);
    float const c {1.0f/std::tan(field_of_view/2)};

   // Match glm::perspective, with GLM_FORCE_DEPTH_ZERO_TO_ONE undefined.
    projection[0][0] = c/aspect_ration;
    projection[1][1] = c;
    projection[2][2] = -(f+n)/(f-n);
    projection[3][2] = -(2*f*n)/(f-n);
    projection[2][3] = -1;

    return projection;
}

glm::mat4 perspective_general_vulkan(float const l, float const r, float const b, float const t, float const n, float const f) {
    glm::mat4 projection = glm::mat4(0.0f);

    projection[0][0] = 2*n/(r-l);
    projection[2][0] = (r+l)/(r-l);
    projection[1][1] = -2*n/(t-b);
    projection[2][1] = -(t+b)/(t-b);
    projection[2][2] = -f/(f-n);
    projection[3][2] = -f*n/(f-n);
    projection[2][3] = -1;

    return projection;
}

/*
// Negate the second column instead of second row, to test if I got the correct matrix.
glm::mat4 perspective_general_vulkan_wrong(float const l, float const r, float const b, float const t, float const n, float const f) {
    glm::mat4 projection = glm::mat4(0.0f);

    projection[0][0] = 2*n/(r-l);
    projection[2][0] = (r+l)/(r-l);
    projection[1][1] = -2*n/(t-b);
    projection[2][1] = (t+b)/(t-b);
    projection[2][2] = -f/(f-n);
    projection[3][2] = -f*n/(f-n);
    projection[2][3] = -1;

    return projection;
}
*/

// field of view is vertical
// aspect ration = width/height
glm::mat4 perspective_vulkan(float const field_of_view, float const aspect_ration, float const n, float const f) {
    glm::mat4 projection = glm::mat4(0.0f);
    float const c {1.0f/std::tan(field_of_view/2)};

    projection[0][0] = c/aspect_ration;
    projection[1][1] = -c;
    projection[2][2] = -f/(f-n);
    projection[3][2] = -f*n/(f-n);
    projection[2][3] = -1;

    return projection;
}

glm::mat4 orthographic_vulkan(float const l, float const r, float const b, float const t, float const n, float const f) {
    glm::mat4 projection = glm::mat4(0.0f);

    projection[0][0] = 2/(r-l);
    projection[3][0] = -(r+l)/(r-l);
    projection[1][1] = -2/(t-b);
    projection[3][1] = (t+b)/(t-b);
    projection[2][2] = -2/(f-n);
    projection[3][2] = -(f+n)/(f-n);
    projection[3][3] = 1.0f;

    return projection;
}

// Rotation used in lookAt function
glm::mat4 lookAtRotation(glm::vec3 const eye, glm::vec3 const center, glm::vec3 const up) {
    // Rotation for camera to look at desired direction
    glm::mat4 rotation {glm::mat4(0.0f)};

    glm::vec3 const v {glm::normalize(eye - center)};
    glm::vec3 const r {glm::normalize(glm::cross(-v, up))};
    glm::vec3 const u {glm::cross(v, r)};

    // Insert r, v and u as rows
    rotation[0][0] = r[0];
    rotation[1][0] = r[1];
    rotation[2][0] = r[2];
    rotation[0][1] = u[0];
    rotation[1][1] = u[1];
    rotation[2][1] = u[2];
    rotation[0][2] = v[0];
    rotation[1][2] = v[1];
    rotation[2][2] = v[2];

    rotation[3][3] = 1.0f;

    return rotation;
}

glm::mat4 lookAt(glm::vec3 const eye, glm::vec3 const center, glm::vec3 const up) {
    // Translation to put camera at the origin
    glm::mat4 const identity {glm::mat4(1.0f)};
    glm::mat4 const translation {glm::translate(identity, -eye)};

    // Rotation for camera to look at desired direction
    glm::mat4 const rotation {transforms::lookAtRotation(eye, center, up)};

    return rotation * translation;
}

}