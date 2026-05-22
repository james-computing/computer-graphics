#pragma once

#include "../include/irenderer.hpp"

#include <iostream>
#include <stdexcept>
#include <cstdint> // For uint32_t
#include <limits> // for std::numeric_limits
#include <algorithm> // for std::clamp
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/gtc/matrix_transform.hpp> // for model view projection
#include <chrono> // for model view projection
#include <unordered_map>

#include "../include/icore.hpp"
#include "../include/window.hpp"
#include "../include/surface.hpp"
#include "../include/swapChain.hpp"
#include "../include/descriptor.hpp"
#include "../include/graphicsPipeline.hpp"
#include "../include/command.hpp"
#include "../include/mvp.hpp"
#include "../include/depthStencil.hpp"
#include "../include/msaa.hpp"
#include "../include/vertexBuffer.hpp"
#include "../include/indexBuffer.hpp"

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

public:
    void initWindow() override;
    void initSurface(vk::raii::Instance const & instance) override;
    void initRest(ICore const & core) override;
    bool step() const override;
    void cleanup() override;

    void drawFrame(std::vector<vk::raii::DescriptorSet> const & descriptorSets, uint32_t const indexCount) override;

    void copyVerticesToVertexBuffer(
        std::vector<Vertex> const & vertices,
        vk::DeviceSize const & dstOffset
    ) const override;
    void copyIndicesToIndexBuffer(
        std::vector<uint32_t> const & indices,
        vk::DeviceSize const & dstOffset
    ) const override;

    void allocateDescriptorSets(
        uint32_t const descriptorSetCount,
        std::vector<vk::raii::DescriptorSet> & descriptorSets
    ) const override;

    vk::raii::SurfaceKHR const & getSurface() const override;

    uint32_t getSwapChainExtentWidth() const override;
    uint32_t getSwapChainExtentHeight() const override;

    uint32_t getMaxFramesInFlight() const override;
    uint32_t getFrameIndex() const override;

    vk::raii::CommandPool const & getCommandPool() const override;

private:
    void transitionImageLayout(
        vk::Image const & image, // not vk::raii::Image, because swapChain.getImages returns vk::Image
        vk::ImageLayout const oldLayout,
        vk::ImageLayout const newLayout,
        vk::AccessFlags2 const srcAccessMask,
        vk::AccessFlags2 const dstAccessMask,
        vk::PipelineStageFlags2 const srcStageMask,
        vk::PipelineStageFlags2 const dstStageMask,
        vk::ImageAspectFlags const imageAspectFlags,
        vk::raii::CommandBuffer const & commandBuffer
    ) const;

    void recordCommandBuffer(
        uint32_t const imageIndex,
        std::vector<vk::raii::DescriptorSet> const & descriptorSets,
        uint32_t const indexCount
    ) const;

    void createSyncObjects(ICore const & core);

    // Group swap chain recreation with color and depth resources recreation
    void recreateSwapChainColorDepth();
};