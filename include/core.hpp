#pragma once

#include "../include/icore.hpp"

#include "../include/validationLayers.hpp"
#include "../include/debugMessenger.hpp"
#include "../include/instance.hpp"
#include "../include/physicalDevice.hpp"
#include "../include/queue.hpp"
#include "../include/device.hpp"
#include "../include/singleTimeCommands.hpp"

class Core : public ICore {
private:
    ///////////////////////////////////////////////// MEMBER VARIABLES //////////////////////////////////

    vk::raii::Context context;
    Instance instance;
    ValidationLayers validationLayers;
    DebugMessenger debugMessenger;
    PhysicalDevice physicalDevice;
    Device device;
    Queue queue;

    /////////////////////////////////////// METHODS //////////////////////////////////////////////////
public:
    void init1() override;
    void init2(vk::raii::SurfaceKHR const & surface) override;

    void copyBufferToImage(
        vk::raii::Buffer const & buffer,
        vk::raii::Image const & image,
        uint32_t const width,
        uint32_t const height,
        vk::raii::Device const & device,
        vk::raii::Queue const & queue,
        vk::raii::CommandPool const & commandPool
    ) const override;

    vk::raii::Instance const & getInstance() const override;
    vk::raii::PhysicalDevice const & getPhysicalDevice() const override;
    vk::raii::Device const & getDevice() const override;
    uint32_t const getQueueFamilyIndex() const override;
    vk::raii::Queue const & getQueue() const override;
};