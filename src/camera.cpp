#include "../include/camera.hpp"

#include <glm/gtc/matrix_transform.hpp> // for glm::translate
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <cstring> // for memcpy
#include <cmath>

#include "../include/ubos.hpp"
#include "../include/transforms.hpp"
#include "../include/buffer.hpp"

void Camera::init(ICore const & core, IInputListener const & inputListener, uint32_t const maxFramesInFlight) {
    inputListenerPtr = &inputListener;

    createUniformBuffers(core, maxFramesInFlight);

    /*
    rotation.x = -45.0f;
    rotation.y = 0.0f;
    rotation.z = 0.0f;]
    */

    location.x = 0.0f;
    location.y = 2.5f;
    location.z = 2.5f;

    // Construct quaternion from euler angles
    //quaternion = glm::quat(glm::vec3(-45.0f, 0.0f, 0.0f));
    theta = -glm::radians(45.0f);
    phi = 0.0f;
    computeCameraAxis();
}

void Camera::createUniformBuffers(ICore const & core, uint32_t const maxFramesInFlight) {
    for (size_t i {0}; i < maxFramesInFlight; ++i) {
        // Create uniform buffer, allocate memory for it and bind it
        vk::DeviceSize constexpr bufferSize {sizeof(CameraUBO)};
        vk::BufferUsageFlags constexpr bufferUsage {vk::BufferUsageFlagBits::eUniformBuffer};
        vk::MemoryPropertyFlags constexpr memoryProperties {
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        };
        vk::raii::Buffer buffer {nullptr};
        vk::raii::DeviceMemory bufferMemory {nullptr};
        Buffer::create(
            core.getPhysicalDevice(),
            core.getDevice(),
            bufferSize,
            bufferUsage,
            memoryProperties,
            buffer,
            bufferMemory
        );
        uniformBuffers.emplace_back(std::move(buffer));
        uniformBuffersMemories.emplace_back(std::move(bufferMemory));
        
        // Map uniform buffer to a pointer, so we can transfer data from the pointer to the uniform buffer
        uniformBuffersMapped.emplace_back(uniformBuffersMemories[i].mapMemory(0, bufferSize));
    }
}

void Camera::computeCameraAxis() {
    // Probably inefficient ...
    /*
    glm::mat4 R {glm::mat4(1.0f)};
    R = glm::rotate(R, rotation.z, glm::vec3(0, 0, 1.0f));
    R = glm::rotate(R, rotation.x, glm::vec3(1.0f, 0, 0));
    R = glm::rotate(R, rotation.y, glm::vec3(0, 1.0f, 0));

    glm::vec4 x4 {glm::vec4(1.0f, 0.0f, 0.0f ,1.0f)};
    glm::vec4 mz4 {glm::vec4(0.0f, 0.0f, -1.0f ,1.0f)}; // camera looks at -z
    glm::vec4 right4 {R * x4};
    glm::vec4 front4 {R * mz4};

    right.x = right4.x;
    right.y = right4.y;
    right.z = right4.z;

    front.x = front4.x;
    front.y = front4.y;
    front.z = front4.z;
    */

    /*
    right = quaternion * glm::vec3(1.0f, 0.0f, 0.0f);
    up = quaternion * glm::vec3(0.0f, 1.0f, 0.0f);
    front = quaternion * glm::vec3(0.0f, 0.0f, -1.0f); // camera looks at -z, if not rotated
    */

    float const cosTheta {cosf(theta)};
    
    front = glm::vec3(
        cosTheta * sinf(phi),
        sinf(theta),
        -cosTheta * cosf(phi)
    );

    right = glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f));
    right = glm::normalize(right);

    up = glm::cross(right, front);
}

void Camera::updateLocation(float const deltaTime, KeysActive const & keysActive) {
    float constexpr translationSpeed {1.8f};
    float step {translationSpeed * deltaTime};

    // Move faster if holding left shift key
    if(keysActive.shift) {
        step *= 2;
    }
    
    // Q moves y down, E moves z up
    if(keysActive.q) {
        location.y -= step;
    }
    if(keysActive.e) {
        location.y += step;
    }

    // W moves camera front, S moves camera back
    if(keysActive.w) {
        location += step * front;
    }
    if(keysActive.s) {
        location -= step * front;
    }

    // A moves camera left, D moves camera right
    if(keysActive.a) {
        location -= step * right;
    }
    if(keysActive.d) {
        location += step * right;
    }   
}

void Camera::updateRotation(float const deltaTime, KeysActive const & keysActive) {
    // Couldn't use the mouse, so use arrow keys instead
    //float constexpr rotationSpeed {20.0f};
    //MouseInput const & mouseInput = inputListenerPtr->getMouseInput();
    //rotation.x += rotationSpeed * mouseInput.dx;
    //rotation.y = rotationSpeed * mouseInput.x;

    float constexpr rotationSpeed {1.5f};
    float const step {rotationSpeed * deltaTime};
    float angleHorizontal {0.0f};
    float angleVertical {0.0f};
    if(keysActive.up) {
        angleVertical = step;
    }
    if(keysActive.down) {
        angleVertical = -step;
    }
    
    if(keysActive.left) {
        angleHorizontal = -step;
    }
    if(keysActive.right) {
        angleHorizontal = step;
    }

    float constexpr maxTheta {glm::radians(89.99f)};
    theta += angleVertical;
    theta = std::clamp(theta, -maxTheta, maxTheta);
    
    phi += angleHorizontal;

    /*
    quaternion = glm::rotate(quaternion, angleVertical, right);
    computeCameraAxis();
    quaternion = glm::rotate(quaternion, angleHorizontal, up);
    */
}

void Camera::updateUniformBuffer(
    uint32_t const frameIndex,
    uint32_t const swapChainExtentWidth,
    uint32_t const swapChainExtentHeight,
    float const deltaTime
) {
    KeysActive const & keysActive {inputListenerPtr->getKeysActive()};
    updateRotation(deltaTime, keysActive);
    computeCameraAxis();
    updateLocation(deltaTime, keysActive);
    //std::cout << "q = (" << quaternion.x << "," << quaternion.y << "," << quaternion.z << "," << quaternion.w << ")" << std::endl;


    // Update the uniform buffer
    CameraUBO ubo;
    ubo.pv = projection(swapChainExtentWidth, swapChainExtentHeight) * view();
    ubo.location = location;

    // Copy the ubo to the corresponding uniform buffer memory.
    // It would be more efficient to use push constants.
    memcpy(uniformBuffersMapped[frameIndex], &ubo, sizeof(ubo));
}

// Transformation that puts camera at origin, looking at -z.
// It is the inverse of getModelMatrix
glm::mat4 Camera::view() const {
    /*
    // Euler angles are inefficient
    // start with identity matrix
    glm::mat4 view {glm::mat4(1.0f)};
    
    view = glm::rotate(view, -glm::radians(rotation.y), glm::vec3(0, 1.0f, 0));
    view = glm::rotate(view, -glm::radians(rotation.x), glm::vec3(1.0f, 0, 0));
    view = glm::rotate(view, -glm::radians(rotation.z), glm::vec3(0, 0, 1.0f));
    */

    /*
    glm::mat4 const rotation {glm::mat4_cast(quaternion)}; // rotation of the camera
    glm::mat4 view = glm::transpose(rotation); // inverse rotation, for the camera to look at -z
    */

    glm::mat4 inverseRotation {glm::mat4(0.0f)};
    inverseRotation[0][0] = right[0];
    inverseRotation[1][0] = right[1];
    inverseRotation[2][0] = right[2];
    inverseRotation[0][1] = up[0];
    inverseRotation[1][1] = up[1];
    inverseRotation[2][1] = up[2];
    inverseRotation[0][2] = -front[0];
    inverseRotation[1][2] = -front[1];
    inverseRotation[2][2] = -front[2];
    inverseRotation[3][3] = 1.0f;

    glm::mat4 view = glm::translate(inverseRotation, -location);

/*
    glm::vec3 constexpr eye {glm::vec3(2.0f, 2.0f, 2.0f)}; // location of the camera
    glm::vec3 constexpr center {glm::vec3(0.0f, 0.0f, 0.0f)}; // location to look at
    glm::vec3 constexpr up {glm::vec3(0.0f, 0.0f, 1.0f)};
    glm::mat4 const view {transforms::lookAt(eye, center, up)};
*/

    //std::cout << "view =\n" << glm::to_string(view) << std::endl;

    return view;
}

glm::mat4 Camera::projection(uint32_t const swapChainExtentWidth, uint32_t const swapChainExtentHeight) const {
    glm::mat4 projection;

    // Perspective projection
    float const aspectRatio {static_cast<float>(swapChainExtentWidth)/static_cast<float>(swapChainExtentHeight)};
    float constexpr field_of_view {45.0f}; // in degrees
    float constexpr near {0.1f};
    float constexpr far {10.f};

    /* Using GLM perspective projection
    // 
    glm::mat4 glmPerspectiveYinverted = glm::perspective(glm::radians(field_of_view), aspectRatio, near, far);
    glmPerspectiveYinverted[1][1] *= -1; // Invert the y
    projection = glmPerspectiveYinverted;
    */

    // Using my implementation of the perspective projection, with field of view.
    projection = transforms::perspective_vulkan(glm::radians(field_of_view), aspectRatio, near, far);

     // General perspective projection
    // Width and height of the camera aren't the same from the swap chain!
    //float const height {2*near*std::tan(glm::radians(field_of_view/2))};
    //float const width {aspectRatio*height};

/*
    float constexpr shift_right {0.05f};
    float constexpr shift_up {0.05f};
    float const r {width/2 +shift_right};
    float const l {-r +shift_right};
    float const t {height/2 +shift_up};
    float const b {-t +shift_up};
    //projection = transforms::perspective_general_vulkan(l, r, b, t, near, far);
*/
    
/*
    float constexpr s {0.01f};
    float const r {s*width/2};
    float const l {-r};
    float const t {s*height/2};
    float const b {-t};
    projection = transforms::orthographic_vulkan(l, r, b, t, near, far);
*/
    //std::cout << "my proj=\n" << glm::to_string(projection) << std::endl;

    return projection;
}