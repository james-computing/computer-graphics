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
    vk::TransformMatrixKHR const identity {
        std::array<std::array<float, 4>, 3>{
            {std::array<float, 4>{1.f, 0.f, 0.f, 0.f},
		    std::array<float, 4>{0.f, 1.f, 0.f, 0.f},
		    std::array<float, 4>{0.f, 0.f, 1.f, 0.f}}}
    };

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

        vk::AccelerationStructureDeviceAddressInfoKHR const addrInfo {
            .accelerationStructure = *blasHandles[i]
        };
        vk::DeviceAddress const blasDeviceAddr {device.getAccelerationStructureAddressKHR(addrInfo)};

        vk::AccelerationStructureInstanceKHR const instance {
            .transform = identity,
            .mask = 0xFF,
            .accelerationStructureReference = blasDeviceAddr
        };

        instances.push_back(instance);

        // Update offsets
        indexDataOffset += modelData.indexCounts[i] * sizeof(uint32_t);
        vertexOffset += modelData.vertexCounts[i];
    }
}

void AccelerationStructures::createTLAS(
    vk::raii::PhysicalDevice const & physicalDevice,
    vk::raii::Device const & device,
    vk::raii::Queue const & queue,
    vk::raii::CommandPool const & commandPool
) {
    // Total number of subMeshes?
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
    uint32_t const numModels
) {
    std::cout << "Create acceleration structures" << std::endl;
    createBLAS(physicalDevice, device, queue, commandPool, modelData, numModels);
    createTLAS(physicalDevice, device, queue, commandPool);
}