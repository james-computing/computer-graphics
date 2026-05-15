#pragma once

#include "core.hpp"
#include "model.hpp"

class Application {
private:
    Core core;
    Model model;
    std::string const modelPath {"./models/viking_room.obj"};
    std::string const texturePath {"./textures/viking_room.png"};
    
    std::vector<vk::raii::Buffer> uniformBuffers; // model view projection matrices are stored in uniform buffers
    std::vector<vk::raii::DeviceMemory> uniformBuffersMemories;
    std::vector<void*> uniformBuffersMapped; // pointers to transfer data from host to uniform buffers

    vk::raii::Sampler textureSampler {nullptr};
    std::vector<vk::raii::DescriptorSet> descriptorSets;
public:
    void run();
private:
    void init(); 
    void createUniformBuffers();
    void updateUniformBuffer(uint32_t frameIndex);

    void allocateDescriptorSets();
    void updateDescriptorSets() const;
    void createDescriptorSets();
};