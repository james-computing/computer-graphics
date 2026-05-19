#include "../include/swapChain.hpp"

vk::SurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const & availableFormats) const {
    auto const formatIterator{
        std::ranges::find_if(
            availableFormats,
            [] (vk::SurfaceFormatKHR const & availableFormat) -> bool {
                return availableFormat.format == vk::Format::eB8G8R8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
            }
        )
    };

    if (formatIterator == availableFormats.end()) {
        return availableFormats[0];
    } else {
        return *formatIterator;
    }
}

vk::PresentModeKHR SwapChain::chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const & availablePresentModes) const {
    bool fifoAvailable {false};
    for (vk::PresentModeKHR const & presentMode : availablePresentModes) {
        switch (presentMode) {
            case vk::PresentModeKHR::eMailbox:
                return vk::PresentModeKHR::eMailbox;
            case vk::PresentModeKHR::eFifo:
                fifoAvailable = true;
                break;
        }
    }

    if (fifoAvailable) {
        return vk::PresentModeKHR::eFifo;
    }
    else {
        throw std::runtime_error("Neither eFifo or eMailbox present modes avaiable"); 
    }
}

vk::Extent2D SwapChain::chooseSwapExtent(vk::SurfaceCapabilitiesKHR const & capabilities, Window const & window) const {
    // If width != max, capabilities.currentExtent already have the correct Extent2D, just return it
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    // Otherwise, we can choose an extent.
    // Width and height must be between the minimum and maximum values allowed, we solve this by clamping.
    // The width and height must be in pixels, the appropriate values are obtained from the framebuffer size.
    int width, height;
    window.getFramebufferSize(width, height);

    return vk::Extent2D {
        std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
}

uint32_t SwapChain::chooseSwapImageCount(vk::SurfaceCapabilitiesKHR const & surfaceCapabilities) const {
    // Pick at least 3 images, and at least the minimum + 1
    uint32_t minImageCount {std::max(3u, surfaceCapabilities.minImageCount + 1)};

    // Don't pass the maximum
    bool const thereIsAMax {surfaceCapabilities.maxImageCount > 0};
    if (thereIsAMax && surfaceCapabilities.maxImageCount < minImageCount) {
        minImageCount = surfaceCapabilities.maxImageCount;
    }

    return minImageCount;
}

void SwapChain::createSwapChain(
    vk::raii::PhysicalDevice const & physicalDevice,
    vk::raii::Device const & device,
    vk::raii::SurfaceKHR const & surface,
    Window const & window
) {
    // Same from createLogicalDevice
    // try catch?
    vk::SurfaceCapabilitiesKHR const surfaceCapabilities {physicalDevice.getSurfaceCapabilitiesKHR(*surface)};
    extent = chooseSwapExtent(surfaceCapabilities, window);
    uint32_t const minImageCount {chooseSwapImageCount(surfaceCapabilities)};

    // Same from createLogicalDevice
    // try catch?
    std::vector<vk::SurfaceFormatKHR> const availableFormats {physicalDevice.getSurfaceFormatsKHR(*surface)};
    surfaceFormat = chooseSwapSurfaceFormat(availableFormats);

    // try catch?
    std::vector<vk::PresentModeKHR> const availablePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);
    vk::PresentModeKHR const presentMode {chooseSwapPresentMode(availablePresentModes)};

    vk::SwapchainCreateInfoKHR const swapChainCreateInfo {
        .surface = *surface,
        .minImageCount = minImageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1, // because not a stereoscopic 3D application
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = surfaceCapabilities.currentTransform, // don't apply any transformation
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = true
    };
    
    // try catch?
    vkraii = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
    // try catch?
    images = vkraii.getImages();
}

void SwapChain::createSwapChainImageViews(vk::raii::Device const & device) {
    assert(imageViews.empty());

    vk::ImageViewCreateInfo imageViewCreateInfo {
        .viewType = vk::ImageViewType::e2D,
        .format = surfaceFormat.format,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    // Could use createImageView in this for loop, but won't, because it could be creating multiple
    // copies of imageViewCreateInfo unnecessarily.
    for (vk::Image & image : images) {
        imageViewCreateInfo.image = image;
        imageViews.emplace_back(vk::raii::ImageView(device, imageViewCreateInfo));
    }
}

void SwapChain::create(
    vk::raii::PhysicalDevice const & physicalDevice,
    vk::raii::Device const & device,
    vk::raii::SurfaceKHR const & surface,
    Window const & window
){
    createSwapChain(physicalDevice, device, surface, window);
    createSwapChainImageViews(device);
}

void SwapChain::cleanupSwapChain(vk::raii::Device const & device) {
    device.waitIdle();
    imageViews.clear();
    vkraii = nullptr;
}

void SwapChain::recreateSwapChain(
    vk::raii::PhysicalDevice const & physicalDevice,
    vk::raii::Device const & device,
    vk::raii::SurfaceKHR const & surface,
    Window const & window
) {
    // Handle window minimization by waiting for width and height to be non zero
    int width;
    int height;
    glfwGetFramebufferSize(window.glfw, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window.glfw, &width, &height);
        glfwWaitEvents();
    }

    // Now the window should not be minimized, with width and height non zero.
    // Proceed with swap chain recreation.

    cleanupSwapChain(device);
    create(physicalDevice, device, surface, window);
}