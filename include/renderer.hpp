#pragma once

#include "irenderer.hpp"

#include <iostream>
#include <stdexcept>
#include <cstdint> // For uint32_t
#include <limits> // for std::numeric_limits
#include <algorithm> // for std::clamp
#include <unordered_map>

#include "icore.hpp"
#include "window.hpp"
#include "surface.hpp"
#include "swapChain.hpp"
#include "descriptor.hpp"
#include "graphicsPipeline.hpp"
#include "command.hpp"
#include "depthStencil.hpp"
#include "msaa.hpp"
//#include "managedBuffer.hpp"
#include "modelData.hpp"
#include "textureSampler.hpp"
#include "indirectDraw.hpp"
#include "accelerationStructures.hpp"

#include "../libraries/stb/stb_image.h"


// Used in drawFrame
struct SyncObjects {
    std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Fence> inFlightFences;
};

class Renderer : public IRenderer {
private:
    ICore const * _corePtr;
    Window window;
    Surface surface;
    SwapChain swapChain;
    GraphicsPipeline graphicsPipeline;
    
    Command command;
    IndirectDraw indirectDraw;

    SyncObjects syncObjects;

    uint32_t const MAX_FRAMES_IN_FLIGHT {2};

    // used by recordCommandBuffer and drawFrame
    uint32_t frameIndex {0};

    uint32_t numModels {0};
    ModelData modelData;
    ModelsInstances modelsInstances;

    Descriptor descriptor;

    DepthStencil depthStencil;
    // multisampling
    MSAA msaa;

    vk::raii::Sampler textureSampler {nullptr};

    AccelerationStructures accelerationStructures;

public:
    void initWindow() override;
    void initSurface(vk::raii::Instance const & instance) override;
    void initRest(ICore const & core) override;
    bool step() const override;
    void cleanup() override;

    void drawFrame(float const deltaTime) override;

    void loadModels(
        std::vector<std::string_view> const & modelPaths,
        std::vector<std::string_view> const & texturePaths,
        std::vector<bool> const & alphaCuts
    ) override;

    vk::raii::SurfaceKHR const & getSurface() const override;

    uint32_t getSwapChainExtentWidth() const override;
    uint32_t getSwapChainExtentHeight() const override;

    uint32_t getMaxFramesInFlight() const override;
    uint32_t getFrameIndex() const override;

    vk::raii::CommandPool const & getCommandPool() const override;

private:
    void recordCommandBuffer(
        uint32_t const imageIndex,
        std::vector<vk::DescriptorSet> const & descriptorSets
    ) const;

    void createSyncObjects(ICore const & core);

    // Group swap chain recreation with color and depth resources recreation
    void recreateSwapChainColorDepth();

public:
    void updateDescriptorSets(
        std::vector<vk::raii::Buffer> const & cameraUniformBuffers
    ) const override;

    Window const & getInputListener();
};