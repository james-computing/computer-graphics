#pragma once

#include "../include/irenderer.hpp"

#include <iostream>
#include <stdexcept>
#include <cstdint> // For uint32_t
#include <limits> // for std::numeric_limits
#include <algorithm> // for std::clamp
#include <unordered_map>

#include "../include/icore.hpp"
#include "../include/window.hpp"
#include "../include/surface.hpp"
#include "../include/swapChain.hpp"
#include "../include/descriptor.hpp"
#include "../include/graphicsPipeline.hpp"
#include "../include/command.hpp"
#include "../include/depthStencil.hpp"
#include "../include/msaa.hpp"
#include "../include/vertexBuffer.hpp"
#include "../include/indexBuffer.hpp"
#include "../include/textureSampler.hpp"

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

    SyncObjects syncObjects;

    uint32_t const MAX_FRAMES_IN_FLIGHT {2};

    // used by recordCommandBuffer and drawFrame
    uint32_t frameIndex {0};

    VertexBuffer vertexBuffer;
    IndexBuffer indexBuffer;

    Descriptor descriptor;

    DepthStencil depthStencil;
    // multisampling
    MSAA msaa;

    vk::raii::Sampler textureSampler {nullptr};

public:
    void initWindow() override;
    void initSurface(vk::raii::Instance const & instance) override;
    void initRest(ICore const & core) override;
    bool step() const override;
    void cleanup() override;

    void drawFrame(uint32_t const indexCount, uint32_t const instanceCount) override;

    void copyVerticesToVertexBuffer(
        std::vector<Vertex> const & vertices,
        size_t const & offset
    ) const override;
    void copyIndicesToIndexBuffer(
        std::vector<uint32_t> const & indices,
        size_t const & offset
    ) const override;

    vk::raii::SurfaceKHR const & getSurface() const override;

    uint32_t getSwapChainExtentWidth() const override;
    uint32_t getSwapChainExtentHeight() const override;

    uint32_t getMaxFramesInFlight() const override;
    uint32_t getFrameIndex() const override;

    vk::raii::CommandPool const & getCommandPool() const override;

private:
    void recordCommandBuffer(
        uint32_t const imageIndex,
        std::vector<vk::DescriptorSet> const & descriptorSets,
        uint32_t const indexCount,
        uint32_t const instanceCount
    ) const;

    void createSyncObjects(ICore const & core);

    // Group swap chain recreation with color and depth resources recreation
    void recreateSwapChainColorDepth();

public:
    void updateDescriptorSets(
        vk::raii::ImageView const & textureImageView,
        std::vector<vk::raii::Buffer> const & cameraUniformBuffers,
        std::vector<vk::raii::Buffer> const & objectUniformBuffers
    ) const override;
};