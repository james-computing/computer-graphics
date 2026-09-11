#pragma once

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>

#include "irenderer.hpp"
#include "iinputListener.hpp"

class Camera {
public:
    glm::vec3 location; // (x, y, z)
    //glm::vec3 rotation; // the 3 euler angles, in degrees
    //glm::quat quaternion;
    float theta; // vertical
    float phi; // horizontal

    std::vector<vk::raii::Buffer> uniformBuffers;

    void init(ICore const & core, IInputListener const & inputListener, uint32_t const maxFramesInFlight);
    glm::mat4 view() const;
    glm::mat4 projection(uint32_t const swapChainExtentWidth, uint32_t const swapChainExtentHeight) const;
    void updateUniformBuffer(
        uint32_t const frameIndex,
        uint32_t const swapChainExtentWidth,
        uint32_t const swapChainExtentHeight,
        float const deltaTime
    );

private:
    std::vector<vk::raii::DeviceMemory> uniformBuffersMemories;
    std::vector<void*> uniformBuffersMapped; // pointers to transfer data from host to uniform buffers

    void createUniformBuffers(ICore const & core, uint32_t const maxFramesInFlight);

    IInputListener const * inputListenerPtr;

    void updateLocation(float const deltaTime, KeysActive const & keysActive);
    void updateRotation(float const deltaTime, KeysActive const & keysActive);

    glm::vec3 right;
    glm::vec3 front;
    glm::vec3 up;

    void computeCameraAxis();
};