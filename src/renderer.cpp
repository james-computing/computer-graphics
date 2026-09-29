#include "../include/renderer.hpp"

#include <thread>
#include <chrono>

void Renderer::initWindow() {
    std::cout << "init window" << std::endl;
    window.init();
}

void Renderer::initSurface(vk::raii::Instance const & instance) {
    std::cout << "init surface" << std::endl;
    surface.create(instance, window.glfwWindow);
}

void Renderer::initRest(ICore const & core) {
    std::cout << "Renderer.initRest" << std::endl;
    // depends on physical device.
    // msaaSamples is used when creating the graphics pipeline and the color and depth resources.
    std::cout << "initMaxUsableSampleCount" << std::endl;
    msaa.initMaxUsableSampleCount(core.getPhysicalDevice());
    std::cout << "initDepthFormat" << std::endl;
    depthStencil.initDepthFormat(core.getPhysicalDevice());

    std::cout << "Create swap chain" << std::endl;
    swapChain.create(core.getPhysicalDevice(), core.getDevice(), surface.vkraii, window);
    
    std::cout << "Create descriptor" << std::endl;
    descriptor.create(core.getDevice(), MAX_FRAMES_IN_FLIGHT);

    std::cout << "Create command" << std::endl;
    command.create(core.getDevice(), core.getQueueFamilyIndex(), MAX_FRAMES_IN_FLIGHT);

    // Depends on logical device, MAX_FRAMES_IN_FLIGHT and swapChainImages.size()
    std::cout << "Create sync objects" << std::endl;
    createSyncObjects(core);

    // depends on descriptorSetLayouts

    std::cout << "Create graphics pipeline" << std::endl;

    // The order is important! It is used by the shader!
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts {
        *descriptor.setLayoutCombinedImageSampler, // set = 0
        *descriptor.setLayoutCamera, // set = 1
        *descriptor.setLayoutModelsInstances, // set = 2
        *descriptor.setLayoutAccelerationStructures // set = 3
    };

    graphicsPipeline.create(
        core.getDevice(),
        swapChain.extent,
        msaa.samples,
        &swapChain.surfaceFormat.format,
        depthStencil.depthFormat,
        descriptorSetLayouts
    );

    // For MSAA. Color resources are used only in recordCommandBuffer.
    msaa.createColorResources(core, swapChain.surfaceFormat.format, swapChain.extent.width, swapChain.extent.height);
    // Depth resources are used only in recordCommandBuffer.
    depthStencil.createDepthResources(core, swapChain.extent.width, swapChain.extent.height, msaa.samples);

    TextureSampler::create(core.getPhysicalDevice(), core.getDevice(), textureSampler);

    _corePtr = &core;
}

void Renderer::loadModels(std::vector<std::string_view> const & modelPaths , std::vector<std::string_view> const texturePaths) {
    std::cout << "Load models" << std::endl;
    numModels = modelPaths.size();
    if(numModels != texturePaths.size()) {
        throw std::runtime_error("Different number of models and textures!");
    }
    std::cout << "numModels = " << numModels << std::endl;

    std::cout << "modelData.init" << std::endl;
    modelData.init(*_corePtr, numModels);

    for (size_t i {0}; i < numModels; ++i) {
        std::cout << "load model " << i << std::endl;
        modelData.load(*_corePtr, command.pool, modelPaths[i], texturePaths[i]);
    }

    std::cout << "create models instances" << std::endl;
    std::vector<uint32_t> instanceCounts;
    instanceCounts.reserve(numModels);
    for (size_t i {0}; i < numModels; ++i) {
        instanceCounts.emplace_back(i+1); // instanceCounts = {1,2,3,...}
    } 
    modelsInstances.init(*_corePtr, MAX_FRAMES_IN_FLIGHT, numModels, instanceCounts);

    std::cout << "Create indirectDraw" << std::endl;
    indirectDraw.create(*_corePtr, MAX_FRAMES_IN_FLIGHT, numModels, modelData, modelsInstances);

    // put somewhere else?
    accelerationStructures.create(
        _corePtr->getPhysicalDevice(),
        _corePtr->getDevice(),
        _corePtr->getQueue(),
        command.pool,
        modelData,
        numModels
    );
}

bool Renderer::step() const {
    if (window.shouldClose()) {
        return false;
    }
    
    window.pollEvents();

    return true;
}

void Renderer::cleanup() {
    swapChain.cleanupSwapChain(_corePtr->getDevice());
    window.cleanup();
}

void Renderer::recordCommandBuffer(
    uint32_t const imageIndex,
    std::vector<vk::DescriptorSet> const & descriptorSets
) const {
    vk::raii::CommandBuffer const & commandBuffer {command.buffers[frameIndex]};

    commandBuffer.begin({});

    // Before start rendering, transition the swap chain image layout to COLOR_ATTACHMENT_OPTIMAL
    Image::transitionImageLayout(
        swapChain.images[imageIndex],
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::AccessFlagBits2::eNone, // don't wait on previous operations
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::ImageAspectFlagBits::eColor,
        commandBuffer
    );

    // Transition multisampled color image to eColorAttachmentOptimal
    Image::transitionImageLayout(
        *msaa.colorImage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::ImageAspectFlagBits::eColor,
        commandBuffer
    );

    // Is it necessary to make this transition for every frame? There is a single transition for the depth buffer.
    Image::transitionImageLayout(
        *depthStencil.depthImage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthAttachmentOptimal,
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::ImageAspectFlagBits::eDepth,
        commandBuffer
    );

    vk::ClearValue constexpr clearColor {vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)}; // black

    // MSAA with resolve
    vk::RenderingAttachmentInfo const colorAttachmentInfo {
        .imageView = *msaa.colorImageView,
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .resolveMode = vk::ResolveModeFlagBits::eAverage,
        .resolveImageView = swapChain.imageViews[imageIndex],
        .resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };

    vk::ClearValue constexpr clearDepth {vk::ClearDepthStencilValue(1.0f, 0)}; // 1.0 = far view plane

    vk::RenderingAttachmentInfo const depthAttachmentInfo {
        .imageView = depthStencil.depthImageView,
        .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .clearValue = clearDepth
    };

    vk::RenderingInfo const renderingInfo {
        .renderArea = vk::Rect2D {
            .offset = {0, 0},
            .extent = swapChain.extent
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentInfo,
        .pDepthAttachment = &depthAttachmentInfo
    };

    // Is it necessary to use a pipeline barrier for indirect drawing?
    /*
    vk::BufferMemoryBarrier2 bufferBarrierIndirectCommand{
        .srcStageMask = vk::PipelineStageFlagBits2::eHost,
        .srcAccessMask = vk::AccessFlagBits2::eHostWrite,
        .dstStageMask = vk::PipelineStageFlagBits2::eDrawIndirect,// | vk::PipelineStageFlagBits2::eAllGraphics,
        .dstAccessMask = vk::AccessFlagBits2::eIndirectCommandRead,// | vk::AccessFlagBits2::eMemoryRead,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .buffer = indirectDraw.buffers[frameIndex],
        .offset = 0,// current frame offset ??
        .size = sizeof(vk::DrawIndexedIndirectCommand)
    };

    vk::DependencyInfo dependencyInfoIndirectCommand {
        .bufferMemoryBarrierCount = 1,
        .pBufferMemoryBarriers = &bufferBarrierIndirectCommand
    };

    commandBuffer.pipelineBarrier2(dependencyInfoIndirectCommand);
    */

    commandBuffer.beginRendering(renderingInfo);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline.vkraii);

    commandBuffer.bindVertexBuffers(0, *modelData.vertexBuffer.buffer, {0});
    
    commandBuffer.bindIndexBuffer(*modelData.indexBuffer.buffer, 0, vk::IndexType::eUint32);

    vk::Viewport const viewport {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(swapChain.extent.width),
        .height = static_cast<float>(swapChain.extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    commandBuffer.setViewport(0, viewport);

    vk::Rect2D const scissor {
        .offset = vk::Offset2D(0, 0),
        .extent = swapChain.extent
    };

    commandBuffer.setScissor(0, scissor);

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        graphicsPipeline.pipelineLayout,
        0,
        descriptorSets,
        nullptr
    );

    // Use drawIndexedIndirect instead of drawIndexed 
    //commandBuffer.drawIndexed(indexCount, instanceCount, 0, 0, 0);

    // Draw the indirect commands
    vk::DeviceSize constexpr offset {0};
    uint32_t const drawCount {numModels}; // number of models
    uint32_t constexpr stride {sizeof(vk::DrawIndexedIndirectCommand)};
    commandBuffer.drawIndexedIndirect(indirectDraw.buffers[frameIndex], offset, drawCount, stride);

    commandBuffer.endRendering();

    // After rendering, transition the swapchain image to PRESENT_SRC
    Image::transitionImageLayout(
        swapChain.images[imageIndex],
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::AccessFlagBits2::eNone,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe,
        vk::ImageAspectFlagBits::eColor,
        commandBuffer
    );

    commandBuffer.end();
}

void Renderer::createSyncObjects(ICore const & core) {
    assert(syncObjects.presentCompleteSemaphores.empty() && syncObjects.renderFinishedSemaphores.empty() && syncObjects.inFlightFences.empty());

    vk::FenceCreateInfo constexpr fenceCreateInfo {
        .flags = vk::FenceCreateFlagBits::eSignaled
    };

    size_t const numberOfImages {swapChain.images.size()};
    for (size_t i {0}; i < numberOfImages; ++i) {
        syncObjects.renderFinishedSemaphores.emplace_back(vk::raii::Semaphore(core.getDevice(), vk::SemaphoreCreateInfo()));
    }

    for (size_t i {0}; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        syncObjects.presentCompleteSemaphores.emplace_back(vk::raii::Semaphore(core.getDevice(), vk::SemaphoreCreateInfo()));
        syncObjects.inFlightFences.emplace_back(vk::raii::Fence(core.getDevice(), fenceCreateInfo));
    }
}

void Renderer::drawFrame(float const deltaTime) {
    modelsInstances.updateShaderStorageBuffer(
        frameIndex,
        deltaTime
    );

    vk::raii::CommandBuffer & commandBuffer {command.buffers[frameIndex]};
    vk::raii::Semaphore & presentCompleteSemaphore {syncObjects.presentCompleteSemaphores[frameIndex]};
    vk::raii::Fence & drawFence {syncObjects.inFlightFences[frameIndex]};

    // Timeout is in nanoseconds. Use UINT64_MAX to effectivelly disable it.
    vk::Result const fenceResult {_corePtr->getDevice().waitForFences(*drawFence, vk::True, UINT64_MAX)};
    if (fenceResult != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for fence");
    }

    // Timeout is in nanoseconds. Use UINT64_MAX to effectivelly disable it.
    vk::ResultValue<uint32_t> const resultValueAcquireNextImage {swapChain.vkraii.acquireNextImage(UINT64_MAX, *presentCompleteSemaphore, {})};
    switch (resultValueAcquireNextImage.result) {
        case vk::Result::eErrorOutOfDateKHR:
        case vk::Result::eSuboptimalKHR: // resultValueAcquireNextImage.has_value() is giving false in this case, so must treat as error
            recreateSwapChainColorDepth();
            return;
        case vk::Result::eSuccess:
            break;
        default:
            throw std::runtime_error("Failed to acquire next image");
    }
    if (!resultValueAcquireNextImage.has_value()) {
        throw std::runtime_error("resultValueAcquireNextImage.has_value() = false");
    }
    uint32_t const imageIndex {resultValueAcquireNextImage.value};

    commandBuffer.reset();

    std::vector<vk::DescriptorSet> descriptorSets {
        *descriptor.setsCombinedImageSampler[frameIndex], //I don't want to send this information to the GPU every frame...
        *descriptor.setsCamera[frameIndex],
        *descriptor.setsModelsInstances[frameIndex],
        *descriptor.setsAccelerationStructures[frameIndex]
    };
    recordCommandBuffer(imageIndex, descriptorSets);

    vk::raii::Semaphore const & renderFinishedSemaphore {syncObjects.renderFinishedSemaphores[imageIndex]}; // imageIndex, not frameIndex
    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    vk::SubmitInfo const submitInfo {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*presentCompleteSemaphore,
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*renderFinishedSemaphore
    };

    _corePtr->getDevice().resetFences(*drawFence);
    _corePtr->getQueue().submit(submitInfo, drawFence);

    vk::PresentInfoKHR const presentInfoKHR {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*renderFinishedSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &*swapChain.vkraii,
        .pImageIndices = &imageIndex,
        .pResults = nullptr // optional
    };

    vk::Result const resultPresent {_corePtr->getQueue().presentKHR(presentInfoKHR)};
    if (resultPresent == vk::Result::eSuboptimalKHR || resultPresent == vk::Result::eErrorOutOfDateKHR || window.getFrameBufferResized()) {
        recreateSwapChainColorDepth();
        return;
    } else if (resultPresent != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to present image");
    }

    /*
    // Slow down to see each frame being rendered
    static int counter = 0;
    std::cout << "frame index = " << frameIndex << ' ' << counter << std::endl;
    ++counter;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    */

    ++frameIndex;
    if (frameIndex == MAX_FRAMES_IN_FLIGHT) {
        frameIndex = 0;
    }
}

// GETTERS

vk::raii::SurfaceKHR const & Renderer::getSurface() const {
    return surface.vkraii;
}

uint32_t Renderer::getSwapChainExtentWidth() const {
    return swapChain.extent.width;
}
    
uint32_t Renderer::getSwapChainExtentHeight() const {
    return swapChain.extent.height;
}

uint32_t Renderer::getMaxFramesInFlight() const {
    return MAX_FRAMES_IN_FLIGHT;
}

uint32_t Renderer::getFrameIndex() const {
    return frameIndex;
}

vk::raii::CommandPool const & Renderer::getCommandPool() const {
    return command.pool;
}

void Renderer::recreateSwapChainColorDepth() {
    swapChain.recreateSwapChain(_corePtr->getPhysicalDevice(), _corePtr->getDevice(), surface.vkraii, window);
    msaa.createColorResources(*_corePtr, swapChain.surfaceFormat.format, swapChain.extent.width, swapChain.extent.height);
    depthStencil.createDepthResources(*_corePtr, swapChain.extent.width, swapChain.extent.height, msaa.samples);
}

void Renderer::updateDescriptorSets(
    std::vector<vk::raii::Buffer> const & cameraUniformBuffers
) const {
    descriptor.updateDescriptorSets(
        _corePtr->getDevice(),
        MAX_FRAMES_IN_FLIGHT,
        textureSampler,
        modelData.textures,
        cameraUniformBuffers,
        modelsInstances.shaderStorageBuffers,
        modelsInstances.getInstanceCountTotal(),
        accelerationStructures.tlas
    );
}

Window const & Renderer::getInputListener() {
    return window;
}