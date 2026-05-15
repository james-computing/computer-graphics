#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
#include "../include/vertex.hpp"
#include "../include/mvp.hpp"
#include "../include/shader.hpp"

#include "../libraries/stb/stb_image.h"

class Core {
private:
    ///////////////////////////////////////////////// MEMBER VARIABLES //////////////////////////////////
    uint32_t const WIDTH {800};
    uint32_t const HEIGHT {600};

    GLFWwindow * window {nullptr};

    vk::raii::Context context;
    vk::raii::Instance instance {nullptr};

    #ifdef NDEBUG
    const bool enableValidationLayers {false};
    #else
    const bool enableValidationLayers {true};
    #endif

    std::vector<char const *> const validationLayers {
        "VK_LAYER_KHRONOS_validation"
    };

    vk::raii::DebugUtilsMessengerEXT debugMessenger {nullptr};

    vk::raii::PhysicalDevice physicalDevice {nullptr};

    vk::raii::Device device {nullptr}; // logical device

    uint32_t queueIndex;
    vk::raii::Queue queue {nullptr};

    vk::raii::SurfaceKHR surface {nullptr};

    vk::Extent2D swapChainExtent;
    vk::SurfaceFormatKHR swapChainSurfaceFormat;
    vk::raii::SwapchainKHR swapChain {nullptr};
    std::vector<vk::Image> swapChainImages;
    std::vector<vk::raii::ImageView> swapChainImageViews;

    vk::raii::PipelineLayout pipelineLayout {nullptr};
    vk::raii::Pipeline graphicsPipeline {nullptr};

    vk::raii::CommandPool commandPool {nullptr};
    std::vector<vk::raii::CommandBuffer> commandBuffers;

    std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Fence> inFlightFences;

public:
    uint32_t const MAX_FRAMES_IN_FLIGHT {2};

private:
    uint32_t frameIndex {0};

    bool frameBufferResized {false};

    size_t MAX_VERTICES {4000};
    size_t MAX_INDICES {12000};
    uint32_t numIndices {0};

    vk::raii::Buffer vertexBuffer {nullptr};
    vk::raii::DeviceMemory vertexBufferMemory {nullptr};
    vk::raii::Buffer indexBuffer {nullptr};
    vk::raii::DeviceMemory indexBufferMemory {nullptr};

    vk::raii::DescriptorSetLayout descriptorSetLayout {nullptr}; // for model view projection, which uses uniform buffers

    vk::raii::DescriptorPool descriptorPool {nullptr};

    vk::raii::Image depthImage {nullptr};
    vk::raii::DeviceMemory depthImageMemory {nullptr};
    vk::raii::ImageView depthImageView {nullptr};
    vk::Format depthFormat;

    vk::SampleCountFlagBits msaaSamples {vk::SampleCountFlagBits::e1};

    // For MSAA
    vk::raii::Image colorImage {nullptr};
    vk::raii::DeviceMemory colorImageMemory {nullptr};
    vk::raii::ImageView colorImageView {nullptr};

    /////////////////////////////////////// METHODS //////////////////////////////////////////////////
public:
    void init();
    bool shouldContinue();
    void pollEvents();
    void cleanup();

private:
    void initWindow();
    void initVulkan();

    std::vector<char const *> getRequiredGLFWExtensions() const;
    std::vector<char const *> getRequiredValidationLayers() const;
    void createInstance();

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT        severity,
        vk::DebugUtilsMessageTypeFlagsEXT               type,
        vk::DebugUtilsMessengerCallbackDataEXT const *  pCallBackData,
        void *                                          pUserData
    );

    void setupDebugMessenger();

    void pickPhysicalDevice();
    bool isDeviceSuitable(vk::raii::PhysicalDevice const & physicalDevice) const;

    void createLogicalDevice();

    void createSurface();

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const & availableFormats) const;
    vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const & availablePresentModes) const;
    vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const & capabilities) const;
    uint32_t chooseSwapImageCount(vk::SurfaceCapabilitiesKHR const & surfaceCapabilities) const;
    void createSwapChain();

    void createSwapChainImageViews();

    void createGraphicsPipeline();

    void createCommandPool();

    void createCommandBuffers();

    void transitionImageLayout(
        vk::Image const & image, // not vk::raii::Image, because swapChain.getImages returns vk::Image
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        vk::AccessFlags2 srcAccessMask,
        vk::AccessFlags2 dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask,
        vk::ImageAspectFlags imageAspectFlags
    ) const;

    void recordCommandBuffer(uint32_t imageIndex, std::vector<vk::raii::DescriptorSet> & descriptorSets);

    void createSyncObjects();

public:
    void drawFrame(std::vector<vk::raii::DescriptorSet> & descriptorSets);

private:
    void cleanupSwapChain();
    void recreateSwapChain();

    static void frameBufferResizeCallback(GLFWwindow * window, int width, int height);

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

public:
    void createBuffer(
        vk::DeviceSize bufferSize,
        vk::BufferUsageFlags bufferUsage,
        vk::MemoryPropertyFlags memoryProperties,
        vk::raii::Buffer & buffer,
        vk::raii::DeviceMemory & bufferMemory
    ) const;

    void copyBuffer(vk::raii::Buffer const & srcBuffer, vk::raii::Buffer const & dstBuffer, vk::DeviceSize bufferSize) const;

private:
    void createVertexBuffer();
    void createIndexBuffer();

public:
    void copyVerticesToVertexBuffer(
        std::vector<Vertex> & vertices
    );
    void copyIndicesToIndexBuffer(
        std::vector<uint32_t> & indices
    );

private:
    void createDescriptorSetLayout();

    void createDescriptorPool();

    void createImage(
        uint32_t width,
        uint32_t height,
        uint32_t mipLevels,
        vk::SampleCountFlagBits numSamples,
        vk::Format imageFormat,
        vk::ImageTiling imageTiling,
        vk::ImageUsageFlags imageUsage,
        vk::MemoryPropertyFlags imageMemoryProperties,
        vk::raii::Image & image,
        vk::raii::DeviceMemory & imageMemory
    );

    void beginSingleTimeCommands(vk::raii::CommandBuffer & commandBuffer) const;
    void endSingleTimeCommands(vk::raii::CommandBuffer const & commandBuffer) const;

    void transitionTextureImageLayout(
        vk::raii::Image const & image,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        uint32_t mipLevels
    ) const;

    void copyBufferToImage(
        vk::raii::Buffer const & buffer,
        vk::raii::Image const & image,
        uint32_t width,
        uint32_t height
    ) const;

    vk::raii::ImageView createImageView(
        vk::raii::Image const & image,
        vk::Format format,
        vk::ImageAspectFlags aspectFlags,
        uint32_t mipLevels
    ) const;

public:

    void createTextureImage(
        int textureWidth,
        int textureHeight,
        stbi_uc * pixels,
        vk::raii::Image & textureImage,
        vk::raii::DeviceMemory & textureImageMemory,
        uint32_t mipLevels
    );
    void createTextureImageView(
        vk::raii::Image & textureImage,
        vk::raii::ImageView & textureImageView,
        uint32_t mipLevels
    );

    void createTextureSampler(vk::raii::Sampler & textureSampler);

private:
    vk::Format findSupportedFormat(
        std::vector<vk::Format> const & candidateFormats,
        vk::ImageTiling tiling,
        vk::FormatFeatureFlags features
    ) const;
    void initDepthFormat();
    bool hasStencilComponent(vk::Format format) const;
    void createDepthResources();

    void generateMipmaps(
        vk::raii::Image & image,
        vk::Format imageFormat,
        int32_t textureWidth,
        int32_t textureHeight,
        uint32_t mipLevels
    ) const;

    void initMaxUsableSampleCount();

    void createColorResources();

    void allocateDescriptorSets(std::vector<vk::raii::DescriptorSet> & descriptorSets) const;

    void updateDescriptorSets(
        std::vector<vk::raii::DescriptorSet> & descriptorSets,
        std::vector<vk::raii::Buffer> & uniformBuffers,
        vk::raii::Sampler & textureSampler,
        vk::raii::ImageView & textureImageView
    ) const;

public:
    void createDescriptorSets(
        std::vector<vk::raii::DescriptorSet> & descriptorSets,
        std::vector<vk::raii::Buffer> & uniformBuffers,
        vk::raii::Sampler & textureSampler,
        vk::raii::ImageView & textureImageView
    ) const;

    uint32_t getSwapChainExtentWidth() const;
    uint32_t getSwapChainExtentHeight() const;

    uint32_t getFrameIndex() const;
};