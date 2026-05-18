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

class DebugMessenger {
private:
    vk::raii::DebugUtilsMessengerEXT vkraii {nullptr};

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT        severity,
        vk::DebugUtilsMessageTypeFlagsEXT               type,
        vk::DebugUtilsMessengerCallbackDataEXT const *  pCallBackData,
        void *                                          pUserData
    );
    
public:
    void setup(vk::raii::Instance const & instance);
};