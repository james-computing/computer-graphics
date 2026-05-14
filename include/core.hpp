#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../include/vertex.hpp"
#include "../include/index.hpp"
#include "../include/shaderModule.hpp"

class Core {
private:
    // MEMBER VARIABLES
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

    bool frameBufferResized {false};

    vk::Extent2D swapChainExtent;
    vk::SurfaceFormatKHR swapChainSurfaceFormat;
    vk::raii::SwapchainKHR swapChain {nullptr};
    std::vector<vk::Image> swapChainImages;
    std::vector<vk::raii::ImageView> swapChainImageViews;

    vk::raii::DescriptorSetLayout descriptorSetLayout {nullptr}; // for model view projection, which uses uniform buffers
    vk::raii::DescriptorPool descriptorPool {nullptr};

    vk::raii::PipelineLayout pipelineLayout {nullptr};
    vk::raii::Pipeline graphicsPipeline {nullptr};

    size_t const MAX_VERTICES {1000};
    vk::raii::Buffer vertexBuffer {nullptr};
    vk::raii::DeviceMemory vertexBufferMemory {nullptr};
    vk::raii::Buffer indexBuffer {nullptr};
    vk::raii::DeviceMemory indexBufferMemory {nullptr};

    vk::raii::CommandPool commandPool {nullptr};
    std::vector<vk::raii::CommandBuffer> commandBuffers;

    std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Fence> inFlightFences;

    uint32_t const MAX_FRAMES_IN_FLIGHT {2};
    uint32_t frameIndex {0};

    // Depth testing
    vk::raii::Image depthImage {nullptr};
    vk::raii::DeviceMemory depthImageMemory {nullptr};
    vk::raii::ImageView depthImageView {nullptr};
    vk::Format depthFormat;

    // For MSAA
    vk::SampleCountFlagBits msaaSamples {vk::SampleCountFlagBits::e1};
    vk::raii::Image colorImage {nullptr};
    vk::raii::DeviceMemory colorImageMemory {nullptr};
    vk::raii::ImageView colorImageView {nullptr};

    // METHODS
public:
    void initVulkan();
    void getPhysicalDeviceProperties(vk::PhysicalDeviceProperties & physicalDeviceProperties) const;
    void getPhysicalDeviceFormatProperties(vk::Format imageFormat, vk::FormatProperties & formatProperties) const;
    vk::raii::Device & getLogicalDevice();
    size_t getMaxFramesInFlight() const;
    uint32_t getSwapChainExtentWidth() const;
    uint32_t getSwapChainExtentHeight() const;
    void getSwapChainExtent(vk::Extent2D & extent) const;
    vk::Format getSwapChainSurfaceFormat() const;
    vk::SampleCountFlagBits getMSAASamples() const;
    vk::Format getDepthFormat() const;
    vk::raii::DescriptorSetLayout & getDescriptorSetLayout();

private:
    void cleanup();
    void initWindow();
    void mainLoop();

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

    static void frameBufferResizeCallback(GLFWwindow * window, int width, int height);

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const & availableFormats) const;
    vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const & availablePresentModes) const;
    vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const & capabilities) const;
    uint32_t chooseSwapImageCount(vk::SurfaceCapabilitiesKHR const & surfaceCapabilities) const;
    void createSwapChain();

    void createSwapChainImageViews();

    void initMaxUsableSampleCount();
    void createColorResources();

    void createDescriptorPool();
    void createDescriptorSetLayout();

    void createGraphicsPipeline();
    void createVertexBuffer();
    void createIndexBuffer();

    void copyVerticesToVertexBuffer(std::vector<Vertex> vertices);
    void copyIndicesToIndexBuffer(std::vector<index_t> indices);

public:
    void createBuffer(
        vk::DeviceSize bufferSize,
        vk::BufferUsageFlags bufferUsage,
        vk::MemoryPropertyFlags memoryProperties,
        vk::raii::Buffer & buffer,
        vk::raii::DeviceMemory & bufferMemory
    );

    void copyBuffer(vk::raii::Buffer const & srcBuffer, vk::raii::Buffer const & dstBuffer, vk::DeviceSize bufferSize) const;

    void copyBufferToImage(
        vk::raii::Buffer const & buffer,
        vk::raii::Image const & image,
        uint32_t width,
        uint32_t height
    ) const;

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

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

    vk::raii::ImageView createImageView(
        vk::raii::Image const & image,
        vk::Format format,
        vk::ImageAspectFlags aspectFlags,
        uint32_t mipLevels
    ) const;

private:
    vk::Format findSupportedFormat(
        std::vector<vk::Format> const & candidateFormats,
        vk::ImageTiling tiling,
        vk::FormatFeatureFlags features
    ) const;
    vk::Format initDepthFormat();
    bool hasStencilComponent(vk::Format format) const;
    void createDepthResources();

    void cleanupSwapChain();
    void recreateSwapChain();

    void createSyncObjects();
    void drawFrame(
        size_t numIndices,
        vk::DescriptorSet & descriptorSet
    );

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

    void recordCommandBuffer(
        uint32_t imageIndex,
        size_t numIndices,
        vk::DescriptorSet & descriptorSet
    );

public:
    void beginSingleTimeCommands(vk::raii::CommandBuffer & commandBuffer) const;
    void endSingleTimeCommands(vk::raii::CommandBuffer const & commandBuffer) const;
};