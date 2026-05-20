#include "../include/core.hpp"

void Core::initVulkan() {
    instance.create(context, validationLayers);

    // depends on instance
    if (validationLayers.enable) {
        debugMessenger.setup(instance.vkraii); // make debug messenger first, because we want to be able to debug early
    }
    physicalDevice.pick(instance.vkraii);
    surface.create(instance.vkraii, window.glfw);

    // depends on physical device.
    // msaaSamples is used when creating the graphics pipeline and the color and depth resources.
    msaa.initMaxUsableSampleCount(physicalDevice.vkraii);
    depthStencil.initDepthFormat(physicalDevice);
    
    // depends on physicalDevice and surface
    device.create(physicalDevice.vkraii, surface.vkraii, queue);

    swapChain.create(physicalDevice.vkraii, device.vkraii, surface.vkraii, window);
    
    // depends on logical device
    createDescriptorSetLayout();
    // depends on logical device and MAX_FRAMES_IN_FLIGHT
    createDescriptorPool();

    // depends on logical device and queueFamilyIndex, but this is obtained when creating the logical device
    createCommandPool();
    // depends on logical device, MAX_FRAMES_IN_FLIGHT and commandPool
    createCommandBuffers();

    // Depends on logical device, MAX_FRAMES_IN_FLIGHT and swapChainImages.size()
    createSyncObjects();

    // depends on descriptorSetLayout

    graphicsPipeline.create(
        device.vkraii,
        swapChain.extent,
        msaa.samples,
        descriptorSetLayout,
        &swapChain.surfaceFormat.format,
        depthStencil.depthFormat
    );
    // For MSAA. Color resources are used only in recordCommandBuffer.
    msaa.createColorResources(*this, swapChain.surfaceFormat.format, swapChain.extent.width, swapChain.extent.height);
    depthStencil.createDepthResources(*this, msaa.samples); // Depth resources are used only in recordCommandBuffer.

    // depends on logical device
    vertexBuffer.create(*this);
    indexBuffer.create(*this);
}

void Core::init() {
    std::cout << "init window" << std::endl;
    window.init();
    std::cout << "initVulkan" << std::endl;
    initVulkan();
}

bool Core::step() const {
    if (window.shouldClose()) {
        return false;
    }
    
    window.pollEvents();

    return true;
}

void Core::cleanup() {
    swapChain.cleanupSwapChain(device.vkraii);
    window.cleanup();
}

// COMMAND BUFFER

void Core::createCommandPool() {
    vk::CommandPoolCreateInfo const commandPoolCreateInfo {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = queue.index
    };

    commandPool = vk::raii::CommandPool(device.vkraii, commandPoolCreateInfo);
}

void Core::createCommandBuffers() {
    vk::CommandBufferAllocateInfo const commandBufferAllocateInfo {
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT
    };

    // vk::raii::CommandBuffers inherits from std::vector<vk::raii:CommandBuffer>
    commandBuffers = vk::raii::CommandBuffers(device.vkraii, commandBufferAllocateInfo);
}

void Core::transitionImageLayout(
    vk::Image const & image,
    vk::ImageLayout const oldLayout,
    vk::ImageLayout const newLayout,
    vk::AccessFlags2 const srcAccessMask,
    vk::AccessFlags2 const dstAccessMask,
    vk::PipelineStageFlags2 const srcStageMask,
    vk::PipelineStageFlags2 const dstStageMask,
    vk::ImageAspectFlags const imageAspectFlags
) const {
    // Use a barrier to change the image layout
    vk::ImageMemoryBarrier2 const barrier {
        .srcStageMask = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = vk::ImageSubresourceRange {
            .aspectMask = imageAspectFlags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    vk::DependencyInfo const dependencyInfo {
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };

    commandBuffers[frameIndex].pipelineBarrier2(dependencyInfo);
}

void Core::recordCommandBuffer(
    uint32_t const imageIndex,
    std::vector<vk::raii::DescriptorSet> const & descriptorSets,
    uint32_t const indexCount
) const {
    vk::raii::CommandBuffer const & commandBuffer {commandBuffers[frameIndex]};

    commandBuffer.begin({});

    // Before start rendering, transition the swap chain image layout to COLOR_ATTACHMENT_OPTIMAL
    transitionImageLayout(
        swapChain.images[imageIndex],
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::AccessFlagBits2::eNone, // don't wait on previous operations
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::ImageAspectFlagBits::eColor
    );

    // Transition multisampled color image to eColorAttachmentOptimal
    transitionImageLayout(
        *msaa.colorImage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::ImageAspectFlagBits::eColor
    );

    // Is it necessary to make this transition for every frame? There is a single transition for the depth buffer.
    transitionImageLayout(
        *depthStencil.depthImage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthAttachmentOptimal,
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::ImageAspectFlagBits::eDepth
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

    commandBuffer.beginRendering(renderingInfo);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline.vkraii);

    commandBuffer.bindVertexBuffers(0, *vertexBuffer.buffer, {0});
    
    commandBuffer.bindIndexBuffer(*indexBuffer.buffer, 0, vk::IndexType::eUint32);

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

    // descriptorSet = *(descriptorSets[frameIndex])
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipeline.pipelineLayout, 0, *(descriptorSets[frameIndex]), nullptr);

    commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);

    commandBuffer.endRendering();

    // After rendering, transition the swapchain image to PRESENT_SRC
    transitionImageLayout(
        swapChain.images[imageIndex],
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::AccessFlagBits2::eNone,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe,
        vk::ImageAspectFlagBits::eColor
    );

    commandBuffer.end();
}

void Core::createSyncObjects() {
    assert(syncObjects.presentCompleteSemaphores.empty() && syncObjects.renderFinishedSemaphores.empty() && syncObjects.inFlightFences.empty());

    vk::FenceCreateInfo constexpr fenceCreateInfo {
        .flags = vk::FenceCreateFlagBits::eSignaled
    };

    size_t const numberOfImages {swapChain.images.size()};
    for (size_t i {0}; i < numberOfImages; ++i) {
        syncObjects.renderFinishedSemaphores.emplace_back(vk::raii::Semaphore(device.vkraii, vk::SemaphoreCreateInfo()));
    }

    for (size_t i {0}; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        syncObjects.presentCompleteSemaphores.emplace_back(vk::raii::Semaphore(device.vkraii, vk::SemaphoreCreateInfo()));
        syncObjects.inFlightFences.emplace_back(vk::raii::Fence(device.vkraii, fenceCreateInfo));
    }
}

void Core::drawFrame(std::vector<vk::raii::DescriptorSet> const & descriptorSets, uint32_t const indexCount) {
    vk::raii::CommandBuffer & commandBuffer {commandBuffers[frameIndex]};
    vk::raii::Semaphore & presentCompleteSemaphore {syncObjects.presentCompleteSemaphores[frameIndex]};
    vk::raii::Fence & drawFence {syncObjects.inFlightFences[frameIndex]};

    // Timeout is in nanoseconds. Use UINT64_MAX to effectivelly disable it.
    vk::Result const fenceResult {device.vkraii.waitForFences(*drawFence, vk::True, UINT64_MAX)};
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
    recordCommandBuffer(imageIndex, descriptorSets, indexCount);

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

    device.vkraii.resetFences(*drawFence);
    queue.vkraii.submit(submitInfo, drawFence);

    vk::PresentInfoKHR const presentInfoKHR {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*renderFinishedSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &*swapChain.vkraii,
        .pImageIndices = &imageIndex,
        .pResults = nullptr // optional
    };

    vk::Result const resultPresent {queue.vkraii.presentKHR(presentInfoKHR)};
    if (resultPresent == vk::Result::eSuboptimalKHR || resultPresent == vk::Result::eErrorOutOfDateKHR || window.getFrameBufferResized()) {
        recreateSwapChainColorDepth();
        return;
    } else if (resultPresent != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to present image");
    }

    ++frameIndex;
    if (frameIndex == MAX_FRAMES_IN_FLIGHT) {
        frameIndex = 0;
    }
}

void Core::createBuffer(
    vk::DeviceSize const bufferSize,
    vk::BufferUsageFlags const bufferUsage,
    vk::MemoryPropertyFlags const memoryProperties,
    vk::raii::Buffer & buffer,
    vk::raii::DeviceMemory & bufferMemory
) const {
    vk::BufferCreateInfo const bufferCreateInfo {
        .size = bufferSize,
        .usage = bufferUsage,
        .sharingMode = vk::SharingMode::eExclusive
    };

    buffer = vk::raii::Buffer(device.vkraii, bufferCreateInfo);

    vk::MemoryRequirements const memoryRequirements {buffer.getMemoryRequirements()};

    uint32_t const memoryTypeIndex {
        physicalDevice.findMemoryType(
            memoryRequirements.memoryTypeBits,
            memoryProperties
        )
    };
    vk::MemoryAllocateInfo const memoryAllocateInfo {
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = memoryTypeIndex
    };

    bufferMemory = vk::raii::DeviceMemory(device.vkraii, memoryAllocateInfo);

    vk::DeviceSize constexpr memoryOffset {0};
    buffer.bindMemory(*bufferMemory, memoryOffset);
}

void Core::beginSingleTimeCommands(vk::raii::CommandBuffer & commandBuffer) const {
    vk::CommandBufferAllocateInfo const commandBufferAllocateInfo {
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };

    commandBuffer = std::move(device.vkraii.allocateCommandBuffers(commandBufferAllocateInfo).front());

    vk::CommandBufferBeginInfo constexpr commandBufferBeginInfo {
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    };
    commandBuffer.begin(commandBufferBeginInfo);
}

void Core::endSingleTimeCommands(vk::raii::CommandBuffer const & commandBuffer) const {
    commandBuffer.end();

    vk::SubmitInfo const submitInfo {
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffer
    };
    queue.vkraii.submit(submitInfo, {});
    queue.vkraii.waitIdle();
}

void Core::copyBuffer(
    vk::raii::Buffer const & srcBuffer,
    vk::raii::Buffer const & dstBuffer,
    vk::DeviceSize const & dstOffset,
    vk::DeviceSize const bufferSize
) const {
    vk::raii::CommandBuffer commandCopyBuffer {nullptr};
    beginSingleTimeCommands(commandCopyBuffer);

    vk::BufferCopy const region {
        .srcOffset = 0,
        .dstOffset = dstOffset,
        .size = bufferSize
    };

    commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, region);

    endSingleTimeCommands(commandCopyBuffer);
}

void Core::copyVerticesToVertexBuffer(
    std::vector<Vertex> const & vertices,
    vk::DeviceSize const & dstOffset
) const {
    copyToBuffer<Vertex>(vertices, dstOffset, vertexBuffer.buffer);
}

void Core::copyIndicesToIndexBuffer(
    std::vector<uint32_t> const & indices,
    vk::DeviceSize const & dstOffset
) const {
    copyToBuffer<uint32_t>(indices, dstOffset, indexBuffer.buffer);
}

void Core::createImage(
    uint32_t const width,
    uint32_t const height,
    uint32_t const mipLevels,
    vk::SampleCountFlagBits const numSamples,
    vk::Format const imageFormat,
    vk::ImageTiling const imageTiling,
    vk::ImageUsageFlags const imageUsage,
    vk::MemoryPropertyFlags const imageMemoryProperties,
    vk::raii::Image & image,
    vk::raii::DeviceMemory & imageMemory
) const {
    vk::Extent3D const extent {
        .width = width,
        .height = height,
        .depth = 1
    };
    vk::ImageCreateInfo const imageCreateInfo {
        .imageType = vk::ImageType::e2D,
        .format = imageFormat,
        .extent = extent,
        .mipLevels = mipLevels,
        .arrayLayers = 1,
        .samples = numSamples,
        .tiling = imageTiling,
        .usage = imageUsage,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined
    };

    image = vk::raii::Image(device.vkraii, imageCreateInfo);

    // Allocate memory for the image
    vk::MemoryRequirements const memoryRequirements {image.getMemoryRequirements()};
    vk::MemoryAllocateInfo const memoryAllocateInfo {
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = physicalDevice.findMemoryType(memoryRequirements.memoryTypeBits, imageMemoryProperties)
    };
    imageMemory = vk::raii::DeviceMemory(device.vkraii, memoryAllocateInfo);
    // Bind the memory
    image.bindMemory(imageMemory, 0);
}

vk::raii::ImageView Core::createImageView(
    vk::raii::Image const & image,
    vk::Format const format,
    vk::ImageAspectFlags const aspectFlags,
    uint32_t const mipLevels
) const {
    vk::ImageViewCreateInfo const imageViewCreateInfo {
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .subresourceRange = vk::ImageSubresourceRange {
            .aspectMask = aspectFlags,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    return vk::raii::ImageView(device.vkraii, imageViewCreateInfo);
}

void Core::copyBufferToImage(
    vk::raii::Buffer const & buffer,
    vk::raii::Image const & image,
    uint32_t const width,
    uint32_t const height
) const {
    vk::raii::CommandBuffer commandBuffer {nullptr};
    beginSingleTimeCommands(commandBuffer);


    vk::ImageSubresourceLayers constexpr imageSubresource {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .layerCount = 1
    };
    vk::Offset3D constexpr offset3D {
        .x = 0,
        .y = 0,
        .z = 0
    };
    vk::BufferImageCopy const region {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = imageSubresource,
        .imageOffset = offset3D, 
        .imageExtent = vk::Extent3D {
            .width = width,
            .height = height,
            .depth = 1
        }
    };

    commandBuffer.copyBufferToImage(*buffer, *image, vk::ImageLayout::eTransferDstOptimal, region);

    endSingleTimeCommands(commandBuffer);
}

// TEXTURE SAMPLER

void Core::createTextureSampler(vk::raii::Sampler & textureSampler) const {
    vk::PhysicalDeviceProperties physicalDeviceProperties {physicalDevice.vkraii.getProperties()};

    vk::SamplerCreateInfo const samplerCreateInfo {
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .mipLodBias = 0.0f,
        .anisotropyEnable = vk::True,
        .maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy,
        .compareEnable = vk::False,
        .compareOp = vk::CompareOp::eAlways,
        .minLod = 0.0f,
        .maxLod = vk::LodClampNone,
        .borderColor = vk::BorderColor::eIntOpaqueBlack,
        .unnormalizedCoordinates = vk::False
    };

    textureSampler = vk::raii::Sampler(device.vkraii, samplerCreateInfo);
}

// DESCRIPTOR SETS

void Core::createDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding constexpr uboDescriptorSetLayoutBinding {
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = nullptr
    };

    vk::DescriptorSetLayoutBinding constexpr combinedImageSamplerDescriptorSetLayoutBinding {
        .binding = 1,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eFragment,
        .pImmutableSamplers = nullptr
    };

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings {
        uboDescriptorSetLayoutBinding, 
        combinedImageSamplerDescriptorSetLayoutBinding
    };

    vk::DescriptorSetLayoutCreateInfo const descriptorSetLayoutCreateInfo {
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };

    descriptorSetLayout = vk::raii::DescriptorSetLayout(device.vkraii, descriptorSetLayoutCreateInfo);
}

void Core::createDescriptorPool() {
    vk::DescriptorPoolSize const uniformBufferDescriptorPoolSize {
        .type = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = MAX_FRAMES_IN_FLIGHT
    };

    vk::DescriptorPoolSize const combinedImageSamplerDescriptorPoolSize {
        .type = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = MAX_FRAMES_IN_FLIGHT
    };

    std::array<vk::DescriptorPoolSize, 2> descriptorPoolSizes {
        uniformBufferDescriptorPoolSize,
        combinedImageSamplerDescriptorPoolSize
    };

    vk::DescriptorPoolCreateInfo const descriptorPoolCreateInfo {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = MAX_FRAMES_IN_FLIGHT,
        .poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
        .pPoolSizes = descriptorPoolSizes.data()
    };

    descriptorPool = vk::raii::DescriptorPool(device.vkraii, descriptorPoolCreateInfo);
}

void Core::allocateDescriptorSets(
    uint32_t const descriptorSetCount,
    std::vector<vk::raii::DescriptorSet> & descriptorSets
) const {
    // Vector with descriptorSetCount copies of *descriptorSetLayout.
    // It is needed because descriptorSetAllocateInfo receives an array of layouts.
    std::vector<vk::DescriptorSetLayout> const descriptorSetLayouts {std::vector(descriptorSetCount, *descriptorSetLayout)};

    // Allocate descriptor sets
    vk::DescriptorSetAllocateInfo const descriptorSetAllocateInfo {
        .descriptorPool = descriptorPool,
        .descriptorSetCount = descriptorSetCount,
        .pSetLayouts = descriptorSetLayouts.data()
    };

    descriptorSets = device.vkraii.allocateDescriptorSets(descriptorSetAllocateInfo);
}

void Core::updateDescriptorSets(std::vector<vk::WriteDescriptorSet> const & writeDescriptorSets) const {
    device.vkraii.updateDescriptorSets(writeDescriptorSets, {});
}

// GETTERS

uint32_t Core::getSwapChainExtentWidth() const {
    return swapChain.extent.width;
}
    
uint32_t Core::getSwapChainExtentHeight() const {
    return swapChain.extent.height;
}

uint32_t Core::getMaxFramesInFlight() const {
    return MAX_FRAMES_IN_FLIGHT;
}

uint32_t Core::getFrameIndex() const {
    return frameIndex;
}

vk::FormatProperties Core::getFormatProperties(vk::Format const imageFormat) const {
    return physicalDevice.vkraii.getFormatProperties(imageFormat);
}

void Core::recreateSwapChainColorDepth() {
    swapChain.recreateSwapChain(physicalDevice.vkraii, device.vkraii, surface.vkraii, window);
    msaa.createColorResources(*this, swapChain.surfaceFormat.format, swapChain.extent.width, swapChain.extent.height);
    depthStencil.createDepthResources(*this, msaa.samples);
}