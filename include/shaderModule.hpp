#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <fstream> // read shader file

class ShaderModule {
public:
    static std::vector<char> readFile(std::string const & filename);
    [[nodiscard]] static vk::raii::ShaderModule create(std::vector<char> const & code, vk::raii::Device const & device);
};