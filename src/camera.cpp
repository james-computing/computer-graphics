#include "../include/camera.hpp"

#include <glm/gtc/matrix_transform.hpp> // for glm::translate
#include <iostream>
#include <cstring> // for memcpy

#include "../include/ubos.hpp"
#include "../include/transforms.hpp"
#include "../include/buffer.hpp"

void Camera::init(ICore const & core, uint32_t const maxFramesInFlight) {
    createUniformBuffers(core, maxFramesInFlight);

    //location = glm::vec3(0, 0, 0);
    //rotation = glm::vec3(0, 0, 0);

    rotation.x = -45.0f;
    rotation.y = 0.0f;
    rotation.z = 0.0f;

    location.x = 0.0f;
    location.y = 2.5f;
    location.z = 2.5f;
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

void Camera::updateUniformBuffer(
    uint32_t const frameIndex,
    uint32_t const swapChainExtentWidth,
    uint32_t const swapChainExtentHeight
) {
    // Update the uniform buffer
    CameraUBO ubo;
    ubo.pv = projection(swapChainExtentWidth, swapChainExtentHeight) * view();

    // Copy the ubo to the corresponding uniform buffer memory.
    // It would be more efficient to use push constants.
    memcpy(uniformBuffersMapped[frameIndex], &ubo, sizeof(ubo));
}

/*
glm::mat4 Camera::getModelMatrix() const {
    // start with identity matrix
    glm::mat4 model {glm::mat4(1.0f)};

    model = glm::translate(model, location);

    model = glm::rotate(model, rotation.z, glm::vec3(0, 0, 1.0f));
    model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0, 0));
    model = glm::rotate(model, rotation.y, glm::vec3(0, 1.0f, 0));

    return model;
}
*/

// Transformation that puts camera at origin, looking at -z.
// It is the inverse of getModelMatrix
glm::mat4 Camera::view() const {
    // start with identity matrix
    glm::mat4 view {glm::mat4(1.0f)};

    view = glm::rotate(view, -glm::radians(rotation.y), glm::vec3(0, 1.0f, 0));
    view = glm::rotate(view, -glm::radians(rotation.x), glm::vec3(1.0f, 0, 0));
    view = glm::rotate(view, -glm::radians(rotation.z), glm::vec3(0, 0, 1.0f));

    view = glm::translate(view, -location);

/*
    glm::vec3 constexpr eye {glm::vec3(2.0f, 2.0f, 2.0f)}; // location of the camera
    glm::vec3 constexpr center {glm::vec3(0.0f, 0.0f, 0.0f)}; // location to look at
    glm::vec3 constexpr up {glm::vec3(0.0f, 0.0f, 1.0f)};
    glm::mat4 const view {transforms::lookAt(eye, center, up)};
*/
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