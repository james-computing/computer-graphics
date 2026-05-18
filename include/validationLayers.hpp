#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

class ValidationLayers {
public:
#ifdef NDEBUG
    const bool enable {false};
#else
    const bool enable {true};
#endif

    std::vector<char const *> getRequiredValidationLayers(vk::raii::Context const & context) const;

private:
    std::vector<char const *> const layers {
        "VK_LAYER_KHRONOS_validation"
    };
};