#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include "../include/window.hpp"

class SwapChain {
public:
    vk::Extent2D extent;
    vk::SurfaceFormatKHR surfaceFormat;
    vk::raii::SwapchainKHR vkraii {nullptr};
    std::vector<vk::Image> images;
    std::vector<vk::raii::ImageView> imageViews;

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const & availableFormats) const;
    vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const & availablePresentModes) const;
    vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const & capabilities, Window const & window) const;
    uint32_t chooseSwapImageCount(vk::SurfaceCapabilitiesKHR const & surfaceCapabilities) const;
    void create(
        vk::raii::PhysicalDevice const & physicalDevice,
        vk::raii::Device const & device,
        vk::raii::SurfaceKHR const & surface,
        Window const & window
    );

    void cleanupSwapChain(vk::raii::Device const & device);
    void recreateSwapChain(
        vk::raii::PhysicalDevice const & physicalDevice,
        vk::raii::Device const & device,
        vk::raii::SurfaceKHR const & surface,
        Window const & window
    );

private:
    void createSwapChain(
        vk::raii::PhysicalDevice const & physicalDevice,
        vk::raii::Device const & device,
        vk::raii::SurfaceKHR const & surface,
        Window const & window
    );
    void createSwapChainImageViews(vk::raii::Device const & device);
};