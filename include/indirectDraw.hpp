#pragma once

#include "icore.hpp"
#include "modelData.hpp"
#include "modelsInstances.hpp"

class IndirectDraw {
public:
    std::vector<vk::raii::Buffer> buffers;

    void create(
        ICore const & core,
        uint32_t const maxFramesInFlight,
        size_t const numModels,
        ModelData const & modelData,
        ModelsInstances const & modelsInstances
    );

private:
    std::vector<vk::raii::DeviceMemory> buffersMemories;
    std::vector<vk::DrawIndexedIndirectCommand> drawIndexedIndirectCommands;

    void createIndirectCommands(
        ICore const & core,
        size_t const numModels,
        ModelData const & modelData,
        ModelsInstances const & modelsInstances
    );
    void createBuffers(ICore const & core, uint32_t const maxFramesInFlight, size_t const numModels);
};