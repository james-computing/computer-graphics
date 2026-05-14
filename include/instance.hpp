#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
//#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <vector>
#include <chrono> // for model view projection

#include "core.hpp"
#include "mvp.hpp"

class Instance {
private:
    std::vector<vk::raii::Buffer> uniformBuffers; // model view projection matrices are stored in uniform buffers
    std::vector<vk::raii::DeviceMemory> uniformBuffersMemories;
    std::vector<void*> uniformBuffersMapped; // pointers to transfer data from host to uniform buffers

    std::vector<vk::raii::DescriptorSet> descriptorSets;

    void init(Core & core);
    void createUniformBuffers(Core & core);
    void updateUniformBuffer(Core & core, uint32_t currentImage);
    void createDescriptorSets(Core & core);
};