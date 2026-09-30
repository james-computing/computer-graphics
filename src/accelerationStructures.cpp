#include "../include/accelerationStructures.hpp"

#include "../include/singleTimeCommands.hpp"
#include "../include/buffer.hpp"

void AccelerationStructures::createBLAS(
    vk::raii::PhysicalDevice const & physicalDevice,
    vk::raii::Device const & device,
    vk::raii::Queue const & queue,
    vk::raii::CommandPool const & commandPool,
    ModelData const & modelData,
    uint32_t const numModels
) {
    vk::BufferDeviceAddressInfo const vertexBufferAddressInfo {
        .buffer = *modelData.vertexBuffer.buffer
    };
    // Use getBufferAddress instead of getBufferAddressKHR, because it has a better error message
    vk::DeviceAddress const vertexBufferAddress {device.getBufferAddress(vertexBufferAddressInfo)};

    vk::BufferDeviceAddressInfo const indexBufferAddressInfo {
        .buffer = *modelData.indexBuffer.buffer
    };
    vk::DeviceAddress const indexBufferAddress {device.getBufferAddress(indexBufferAddressInfo)};

    uint32_t indexDataOffset {0};
    uint32_t vertexOffset {0};
    for (size_t i {0}; i < numModels; ++i) {
        vk::AccelerationStructureGeometryTrianglesDataKHR const trianglesData {
            .vertexFormat = Vertex::format,
            .vertexData = vertexBufferAddress,
            .vertexStride = sizeof(Vertex),
            .maxVertex = modelData.vertexCounts[i],
            .indexType = vk::IndexType::eUint32, // same used in commanBuffer.bindIndexBuffer
            .indexData = indexBufferAddress + indexDataOffset
        };

        vk::AccelerationStructureGeometryDataKHR const geometryData {vk::AccelerationStructureGeometryDataKHR(trianglesData)};

        vk::AccelerationStructureGeometryKHR const blasGeometry {
            .geometryType = vk::GeometryTypeKHR::eTriangles,
            .geometry = geometryData,
            .flags = vk::GeometryFlagBitsKHR::eOpaque
        };

        // Can't be const, because of scratchData, which will be set later
        vk::AccelerationStructureBuildGeometryInfoKHR blasBuildGeometryInfo {
            .type = vk::AccelerationStructureTypeKHR::eBottomLevel,
            .mode = vk::BuildAccelerationStructureModeKHR::eBuild,
            .geometryCount = 1,
            .pGeometries = &blasGeometry
        };

        uint32_t const primitiveCount {modelData.indexCounts[i] / 3}; // number of triangles
        std::vector<uint32_t> const maxPrimitiveCounts {primitiveCount}; 
        vk::AccelerationStructureBuildSizesInfoKHR const blasBuildSizes {
            device.getAccelerationStructureBuildSizesKHR(
                vk::AccelerationStructureBuildTypeKHR::eDevice,
                blasBuildGeometryInfo,
                maxPrimitiveCounts
            )
        };

        // Create scratch buffer
        vk::raii::Buffer blasScratchBuffer {nullptr};
        vk::raii::DeviceMemory blasScratchBufferMemory {nullptr};
        vk::BufferUsageFlags constexpr blasScratchBufferUsage {
            vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress
        };
        vk::MemoryPropertyFlags constexpr blasScratchBufferMemoryProperties {vk::MemoryPropertyFlagBits::eDeviceLocal};
        Buffer::create(
            physicalDevice,
            device,
            blasBuildSizes.buildScratchSize,
            blasScratchBufferUsage,
            blasScratchBufferMemoryProperties,
            blasScratchBuffer,
            blasScratchBufferMemory
        );

        vk::BufferDeviceAddressInfo const blasScratchBufferAddressInfo {.buffer = *blasScratchBuffer};
        vk::DeviceAddress const blasScratchBufferAddress {device.getBufferAddress(blasScratchBufferAddressInfo)};
        blasBuildGeometryInfo.scratchData.deviceAddress = blasScratchBufferAddress;

        vk::raii::Buffer blasBuffer {nullptr};
        vk::raii::DeviceMemory blasBufferMemory {nullptr};
        vk::BufferUsageFlags constexpr blasBufferUsage {
            vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
            vk::BufferUsageFlagBits::eShaderDeviceAddress |
            vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
        };
        vk::MemoryPropertyFlags constexpr blasMemoryProperties {vk::MemoryPropertyFlagBits::eDeviceLocal};
        Buffer::create(
            physicalDevice,
            device,
            blasBuildSizes.accelerationStructureSize,
            blasBufferUsage,
            blasMemoryProperties,
            blasBuffer,
            blasBufferMemory
        );
        blasBuffers.emplace_back(std::move(blasBuffer));
        blasBufferMemories.emplace_back(std::move(blasBufferMemory));

        vk::AccelerationStructureCreateInfoKHR const blasCreateInfo {
            .buffer = *blasBuffers[i],
            .offset = 0,
            .size = blasBuildSizes.accelerationStructureSize,
            .type = vk::AccelerationStructureTypeKHR::eBottomLevel
        };

        blasHandles.emplace_back(device.createAccelerationStructureKHR(blasCreateInfo));
        blasBuildGeometryInfo.dstAccelerationStructure = blasHandles[i];

        vk::AccelerationStructureBuildRangeInfoKHR const blasRangeInfo {
            .primitiveCount = primitiveCount,
            .primitiveOffset = 0,
            .firstVertex = vertexOffset,
            .transformOffset = 0
        };

        vk::raii::CommandBuffer commandBuffer {nullptr};
        SingleTimeCommands::begin(device, commandPool, commandBuffer);
        commandBuffer.buildAccelerationStructuresKHR({blasBuildGeometryInfo}, {&blasRangeInfo});
        SingleTimeCommands::end(queue, commandBuffer);

        // Update offsets
        indexDataOffset += modelData.indexCounts[i] * sizeof(uint32_t);
        vertexOffset += modelData.vertexCounts[i];
    }
}

void AccelerationStructures::createInstances(
    vk::raii::Device const & device,
    ModelsInstances const & modelsInstances,
    uint32_t const numModels
) {
    vk::TransformMatrixKHR const identity {
        std::array<std::array<float, 4>, 3>{
            {std::array<float, 4>{1.f, 0.f, 0.f, 0.f},
		    std::array<float, 4>{0.f, 1.f, 0.f, 0.f},
		    std::array<float, 4>{0.f, 0.f, 1.f, 0.f}}}
    };

    uint32_t const totalInstances {modelsInstances.getInstanceCountTotal()};
    instances.reserve(totalInstances);
    for (size_t i {0}; i < numModels; ++i) {
        vk::AccelerationStructureDeviceAddressInfoKHR const addrInfo {
            .accelerationStructure = *blasHandles[i]
        };
        vk::DeviceAddress const blasDeviceAddr {device.getAccelerationStructureAddressKHR(addrInfo)};

        uint32_t const instanceCount {modelsInstances.getInstanceCount(i)};
        for (size_t j {0}; j < instanceCount; ++j) {
            vk::AccelerationStructureInstanceKHR const instance {
                .transform = identity, // should replace with the transform for the instance
                .mask = 0xFF,
                .accelerationStructureReference = blasDeviceAddr
            };

            instances.push_back(instance);
        }
    }
}

void AccelerationStructures::createTLAS(
    vk::raii::PhysicalDevice const & physicalDevice,
    vk::raii::Device const & device,
    vk::raii::Queue const & queue,
    vk::raii::CommandPool const & commandPool
) {
    // Total number of models
    size_t const instancesSize {instances.size()};
    vk::DeviceSize const instanceBufferSize {instancesSize * sizeof(vk::AccelerationStructureInstanceKHR)};
    vk::BufferUsageFlags constexpr instanceBufferUsage {
        vk::BufferUsageFlagBits::eShaderDeviceAddress |
        vk::BufferUsageFlagBits::eTransferDst |
        vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
    };
    vk::MemoryPropertyFlags constexpr instanceBufferMemoryProperties {
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent
    };
    Buffer::create(
        physicalDevice,
        device,
        instanceBufferSize,
        instanceBufferUsage,
        instanceBufferMemoryProperties,
        instanceBuffer,
        instanceBufferMemory
    );

    void * data {instanceBufferMemory.mapMemory(0, instanceBufferSize)};
    memcpy(data, instances.data(), instanceBufferSize);
    instanceBufferMemory.unmapMemory();

    vk::BufferDeviceAddressInfo const instanceAddressInfo {
        .buffer = instanceBuffer
    };
    vk::DeviceAddress const instanceAddress {device.getBufferAddress(instanceAddressInfo)};

    vk::AccelerationStructureGeometryInstancesDataKHR const instancesData {
        .arrayOfPointers = vk::False,
        .data = instanceAddress
    };

    vk::AccelerationStructureGeometryDataKHR const geometryData {instancesData};

    vk::AccelerationStructureGeometryKHR const tlasGeometry {
        .geometryType = vk::GeometryTypeKHR::eInstances,
        .geometry = geometryData
    };

    // Can't be const, because we'll edit the scratchData later
    vk::AccelerationStructureBuildGeometryInfoKHR tlasBuildGeometryInfo {
        .type = vk::AccelerationStructureTypeKHR::eTopLevel,
        .flags = vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate,
        .mode = vk::BuildAccelerationStructureModeKHR::eBuild,
        .geometryCount = 1,
        .pGeometries = &tlasGeometry
    };

    vk::AccelerationStructureBuildSizesInfoKHR const tlasBuildSizes {
        device.getAccelerationStructureBuildSizesKHR(
            vk::AccelerationStructureBuildTypeKHR::eDevice,
            tlasBuildGeometryInfo,
            {static_cast<uint32_t>(instancesSize)} // maxPrimitiveCounts
        )
    };

    vk::BufferUsageFlags constexpr tlasScratchBufferUsage {
        vk::BufferUsageFlagBits::eStorageBuffer |
        vk::BufferUsageFlagBits::eShaderDeviceAddress
    };
    vk::MemoryPropertyFlags constexpr tlasScratchBufferMemoryProperties {
        vk::MemoryPropertyFlagBits::eDeviceLocal
    };
    Buffer::create(
        physicalDevice,
        device,
        tlasBuildSizes.buildScratchSize,
        tlasScratchBufferUsage,
        tlasScratchBufferMemoryProperties,
        tlasScratchBuffer,
        tlasScratchBufferMemory
    );

    vk::BufferDeviceAddressInfo const tlasScratchBufferAddressInfo {
        .buffer = *tlasScratchBuffer
    };
    vk::DeviceAddress const tlasScratchAddress {device.getBufferAddress(tlasScratchBufferAddressInfo)};
    tlasBuildGeometryInfo.scratchData.deviceAddress = tlasScratchAddress;

    vk::BufferUsageFlags constexpr tlasBufferUsage {
        vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
        vk::BufferUsageFlagBits::eShaderDeviceAddress |
        vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
    };
    vk::MemoryPropertyFlags constexpr tlasBufferMemoryProperties {
        vk::MemoryPropertyFlagBits::eDeviceLocal
    };
    Buffer::create(
        physicalDevice,
        device,
        tlasBuildSizes.accelerationStructureSize,
        tlasBufferUsage,
        tlasBufferMemoryProperties,
        tlasBuffer,
        tlasBufferMemory
    );

    vk::AccelerationStructureCreateInfoKHR const tlasCreateInfo {
        .buffer = *tlasBuffer,
        .offset = 0,
        .size = tlasBuildSizes.accelerationStructureSize,
        .type = vk::AccelerationStructureTypeKHR::eTopLevel
    };
    tlas = device.createAccelerationStructureKHR(tlasCreateInfo);

    tlasBuildGeometryInfo.dstAccelerationStructure = tlas;

    vk::AccelerationStructureBuildRangeInfoKHR const tlasRangeInfo {
        .primitiveCount = static_cast<uint32_t>(instancesSize),
        .primitiveOffset = 0,
        .firstVertex = 0,
        .transformOffset = 0
    };

    vk::raii::CommandBuffer commandBuffer {nullptr};
    SingleTimeCommands::begin(device, commandPool, commandBuffer);

    commandBuffer.buildAccelerationStructuresKHR({tlasBuildGeometryInfo},{&tlasRangeInfo});

    SingleTimeCommands::end(queue, commandBuffer);
}

void AccelerationStructures::create(
    vk::raii::PhysicalDevice const & physicalDevice,
    vk::raii::Device const & device,
    vk::raii::Queue const & queue,
    vk::raii::CommandPool const & commandPool,
    ModelData const & modelData,
    uint32_t const numModels,
    ModelsInstances const & modelsInstances
) {
    std::cout << "Create acceleration structures" << std::endl;
    createBLAS(physicalDevice, device, queue, commandPool, modelData, numModels);
    createInstances(device, modelsInstances, numModels);
    createTLAS(physicalDevice, device, queue, commandPool);
}

void AccelerationStructures::updateTLAS(
    vk::raii::PhysicalDevice const & physicalDevice,
    vk::raii::Device const & device,
    vk::raii::Queue const & queue,
    vk::raii::CommandPool const & commandPool,
    std::vector<glm::mat4> const & modelMatrices
) {
    uint32_t const instancesSize {static_cast<uint32_t>(instances.size())};
    if (instancesSize != modelMatrices.size()) {
        std::cout << "instancesSize = " << instancesSize << std::endl;
        std::cout << "modelMatrices.size() = " << modelMatrices.size() << std::endl;
        throw std::runtime_error("modelMatrices should have size = instancesSize.");
    }

    for (size_t i {0}; i < instancesSize; ++i) {
        glm::mat4 const & modelMatrix {modelMatrices[i]};
        vk::TransformMatrixKHR transform {};
        transform.matrix = std::array<std::array<float,4>,3>{{
            std::array<float,4>{modelMatrix[0][0], modelMatrix[1][0], modelMatrix[2][0], modelMatrix[3][0]},
            std::array<float,4>{modelMatrix[0][1], modelMatrix[1][1], modelMatrix[2][1], modelMatrix[3][1]},
            std::array<float,4>{modelMatrix[0][2], modelMatrix[1][2], modelMatrix[2][2], modelMatrix[3][2]}
        }};
        instances[i].setTransform(transform);
    }

    vk::DeviceSize const instanceBufferSize {instancesSize * sizeof(vk::AccelerationStructureInstanceKHR)};

    vk::raii::Buffer stagingBuffer {nullptr};
    vk::raii::DeviceMemory stagingBufferMemory {nullptr};
    vk::BufferUsageFlags stagingBufferUsage {vk::BufferUsageFlagBits::eTransferSrc};
    vk::MemoryPropertyFlags stagingBufferMemoryProperties {
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    };
    Buffer::create(
        physicalDevice,
        device,
        instanceBufferSize,
        stagingBufferUsage,
        stagingBufferMemoryProperties,
        stagingBuffer,
        stagingBufferMemory
    );

    void * data {stagingBufferMemory.mapMemory(0, instanceBufferSize)};
    memcpy(data, instances.data(), instanceBufferSize);
    stagingBufferMemory.unmapMemory();
    data = nullptr;

    Buffer::copyToBuffer(
        device,
        queue,
        commandPool,
        stagingBuffer,
        instanceBuffer,
        0, // dstOffset
        instanceBufferSize
    );

    vk::BufferDeviceAddressInfo const instanceAddressInfo {
        .buffer = instanceBuffer
    };

    vk::DeviceAddress const instanceAddress {device.getBufferAddress(instanceAddressInfo)};

    vk::AccelerationStructureGeometryInstancesDataKHR const instanceData {
        .arrayOfPointers = vk::False,
        .data = instanceAddress
    };

    vk::AccelerationStructureGeometryDataKHR const geometryData(instanceData);

    vk::AccelerationStructureGeometryKHR const tlasGeometry {
        .geometryType = vk::GeometryTypeKHR::eInstances,
        .geometry = geometryData
    };

    // Can't be const, because will edit the scratchData later
    vk::AccelerationStructureBuildGeometryInfoKHR tlasBuildGeometryInfo {
        .type = vk::AccelerationStructureTypeKHR::eTopLevel,
        .flags = vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate,
        .mode = vk::BuildAccelerationStructureModeKHR::eUpdate,
        .srcAccelerationStructure = *tlas,
        .dstAccelerationStructure = *tlas,
        .geometryCount = 1,
        .pGeometries = &tlasGeometry
    };

    vk::BufferDeviceAddressInfo const tlasScratchBufferAddressInfo {
        .buffer = *tlasScratchBuffer
    };

    vk::DeviceAddress const tlasScratchBufferAddress {device.getBufferAddress(tlasScratchBufferAddressInfo)};

    tlasBuildGeometryInfo.scratchData.deviceAddress = tlasScratchBufferAddress;

    vk::AccelerationStructureBuildRangeInfoKHR const tlasRangeInfo {
        .primitiveCount = instancesSize,
        .primitiveOffset = 0,
        .firstVertex = 0,
        .transformOffset = 0
    };

    vk::raii::CommandBuffer commandBuffer {nullptr};
    SingleTimeCommands::begin(device, commandPool, commandBuffer);

    vk::MemoryBarrier constexpr preBarrier {
        .srcAccessMask = vk::AccessFlagBits::eAccelerationStructureWriteKHR | vk::AccessFlagBits::eTransferWrite | vk::AccessFlagBits::eShaderRead,
        .dstAccessMask = vk::AccessFlagBits::eAccelerationStructureReadKHR | vk::AccessFlagBits::eAccelerationStructureWriteKHR
    };
    vk::PipelineStageFlags constexpr preSrcStageMask {
        vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR |
        vk::PipelineStageFlagBits::eTransfer |
        vk::PipelineStageFlagBits::eFragmentShader
    };
    vk::PipelineStageFlags constexpr preDstStageMask {
        vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR
    };

    // use pipelineBarrier2 instead?
    commandBuffer.pipelineBarrier(
        preSrcStageMask,
        preDstStageMask,
        {},
        {preBarrier},
        {},
        {}
    );

    commandBuffer.buildAccelerationStructuresKHR({tlasBuildGeometryInfo}, {&tlasRangeInfo});

    vk::MemoryBarrier constexpr postBarrier {
        .srcAccessMask = vk::AccessFlagBits::eAccelerationStructureWriteKHR,
        .dstAccessMask = vk::AccessFlagBits::eAccelerationStructureReadKHR | vk::AccessFlagBits::eShaderRead
    };

    vk::PipelineStageFlags constexpr postSrcStageMask {vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR};
    vk::PipelineStageFlags constexpr postDstStageMask {
        vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR | vk::PipelineStageFlagBits::eFragmentShader,
    };

    commandBuffer.pipelineBarrier(
        postSrcStageMask,
        postDstStageMask,
        {},
        {postBarrier},
        {},
        {}
    );

    SingleTimeCommands::end(queue, commandBuffer);
}