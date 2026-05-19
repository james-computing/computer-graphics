#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include "../include/validationLayers.hpp"
#include "../include/glfwExtensions.hpp"

class Instance {
public:
    vk::raii::Instance vkraii {nullptr};

    void create(vk::raii::Context const & context, ValidationLayers const & validationLayers);
};