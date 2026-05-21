#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

namespace SingleTimeCommands {

void begin(vk::raii::Device const & device, vk::raii::CommandPool const & commandPool, vk::raii::CommandBuffer & commandBuffer);
void end(vk::raii::Queue const & queue, vk::raii::CommandBuffer const & commandBuffer);

}