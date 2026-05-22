#pragma once

#include "../include/icore.hpp"
#include "../include/validationLayers.hpp"
#include "../include/debugMessenger.hpp"
#include "../include/instance.hpp"
#include "../include/physicalDevice.hpp"
#include "../include/queue.hpp"
#include "../include/device.hpp"

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

    vk::raii::Instance const & getInstance() const override;
    vk::raii::PhysicalDevice const & getPhysicalDevice() const override;
    vk::raii::Device const & getDevice() const override;
    uint32_t const getQueueFamilyIndex() const override;
    vk::raii::Queue const & getQueue() const override;
};