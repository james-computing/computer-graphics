#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include "../include/memoryType.hpp"

namespace Image {

void create(
    vk::raii::PhysicalDevice const & physicalDevice,
    vk::raii::Device const & device,
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
);

vk::raii::ImageView createView(
    vk::raii::Device const & device,
    vk::raii::Image const & image,
    vk::Format const format,
    vk::ImageAspectFlags const  aspectFlags,
    uint32_t const mipLevels
);

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
);

}