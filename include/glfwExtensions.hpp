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

namespace GLFWExtensions {

std::vector<char const *> getRequiredGLFWExtensions(vk::raii::Context const & context, bool enableValidationLayers);

}