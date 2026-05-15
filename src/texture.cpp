#include "../include/texture.hpp"

void Texture::computeMipLevels(int const textureWidth, int const textureHeight) {
    // sum 1 for the original image to have a mip level
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(textureWidth, textureHeight)))) + 1;
}

void Texture::transitionTextureImageLayout(
    Core const & core,
    vk::ImageLayout const oldLayout,
    vk::ImageLayout const newLayout
) const {
    vk::raii::CommandBuffer commandBuffer {nullptr};
    core.beginSingleTimeCommands(commandBuffer);

    vk::ImageMemoryBarrier barrier {
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .image = image,
        .subresourceRange = vk::ImageSubresourceRange {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else {
        throw std::invalid_argument("Unsupported image layout transition");
    }

    commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);

    core.endSingleTimeCommands(commandBuffer);
}

void Texture::generateMipmaps(Core const & core, vk::Format imageFormat) const {
    // Check if linear blitting is supported
    vk::FormatProperties const formatProperties {core.getFormatProperties(imageFormat)};
    if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
        throw std::runtime_error("Texture image support does not support linear blitting!");
    }

    // Proceed with blitting.
    // It works as follows.
    // All mip levels start in layout eTransferDstOptimal, as of entering this function.
    // To blit, we need the level i - 1 to be in the layout eTransferSrcOptimal.
    // A barrier is used to make this transition before blitting.
    // After blitting, the level i - 1 is transitioned to the layout eShaderReadOnlyOptimal,
    // since the texture will be read by the shader in the fragment stage.
    // The last level, which is mipLevels - 1, isn't transitioned to eTransferSrcOptimal, so
    // it is transitioned directly to the layout eShaderReadOnlyOptimal in the end.
    
    vk::raii::CommandBuffer commandBuffer {nullptr};
    core.beginSingleTimeCommands(commandBuffer);

    vk::ImageMemoryBarrier barrier {
        // Redundant, same values will be set in for loop.
        //.srcAccessMask = vk::AccessFlagBits::eTransferWrite,
        //.dstAccessMask = vk::AccessFlagBits::eTransferRead,
        //.oldLayout = vk::ImageLayout::eTransferDstOptimal,
        //.newLayout = vk::ImageLayout::eTransferSrcOptimal,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .image = *image,
        .subresourceRange = vk::ImageSubresourceRange {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    int32_t mipWidth {textureWidth};
    int32_t mipHeight {textureHeight};

    for (uint32_t i {1}; i < mipLevels; ++i) {
        // Transition previous mip level to transfer read
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            {},
            {},
            {},
            barrier
        );

        // Make mip map
        vk::ArrayWrapper1D<vk::Offset3D, 2> srcOffsets;
        vk::ArrayWrapper1D<vk::Offset3D, 2> dstOffsets;
        srcOffsets[0] = vk::Offset3D(0, 0, 0);
        srcOffsets[1] = vk::Offset3D(mipWidth, mipHeight, 1);
        dstOffsets[0] = vk::Offset3D(0, 0, 0);
        dstOffsets[1] = vk::Offset3D(mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1);

        vk::ImageBlit const blit {
            .srcSubresource = vk::ImageSubresourceLayers {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .mipLevel = i - 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .srcOffsets = srcOffsets,
            .dstSubresource = vk::ImageSubresourceLayers {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .mipLevel = i,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .dstOffsets = dstOffsets
        };

        commandBuffer.blitImage(
            *image,
            vk::ImageLayout::eTransferSrcOptimal,
            *image,
            vk::ImageLayout::eTransferDstOptimal,
            blit,
            vk::Filter::eLinear
        );

        // Transition previous mip level to shader read optimal
        barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eFragmentShader,
            {},
            {},
            {},
            barrier
        );

        if (mipWidth > 1) {
            mipWidth /= 2;
        }
        if (mipHeight > 1) {
            mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = i;
    }

    // Transition last mip level to shader read optimal.
    // The last mip level isn't used for blitting, so it doesn't transition to eTransferSrcOptimal,
    // it goes straight from eTransferDstOptimal to eShaderReadOnlyOptimal.
    // barrier.subresourceRange.baseMipLevel = mipLevels-1; // assigned in for loop
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eFragmentShader,
        {},
        {},
        {},
        barrier
    );

    core.endSingleTimeCommands(commandBuffer);
}

void Texture::createTextureImage(Core const & core, stbi_uc const * const pixels) {
    vk::DeviceSize const imageSize {(vk::DeviceSize) (textureWidth * textureHeight * 4)};

    // Create a staging buffer to receive the image
    vk::raii::Buffer stagingBuffer {nullptr};
    vk::raii::DeviceMemory stagingBufferMemory {nullptr};
    vk::BufferUsageFlags constexpr stagingBufferUsageFlags {vk::BufferUsageFlagBits::eTransferSrc};
    vk::MemoryPropertyFlags constexpr stagingBufferMemoryProperties {
        // Memory visible to host and available immediately to the device
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    };
    core.createBuffer(
        imageSize,
        stagingBufferUsageFlags,
        stagingBufferMemoryProperties,
        stagingBuffer,
        stagingBufferMemory
    );

    // Transfer the image to the staging buffer
    void * data {stagingBufferMemory.mapMemory(0, imageSize)};
    memcpy(data, pixels, imageSize);
    stagingBufferMemory.unmapMemory();
    data = nullptr;

    vk::ImageTiling constexpr imageTiling {vk::ImageTiling::eOptimal};
    vk::ImageUsageFlags constexpr imageUsage {
        vk::ImageUsageFlagBits::eTransferSrc | // for mip map creation
        vk::ImageUsageFlagBits::eTransferDst |
        vk::ImageUsageFlagBits::eSampled
    };
    vk::MemoryPropertyFlags constexpr imageMemoryProperties {vk::MemoryPropertyFlagBits::eDeviceLocal};
    core.createImage(
        static_cast<uint32_t>(textureWidth),
        static_cast<uint32_t>(textureHeight),
        mipLevels,
        vk::SampleCountFlagBits::e1,
        vk::Format::eR8G8B8A8Srgb,
        imageTiling,
        imageUsage,
        imageMemoryProperties,
        image,
        imageMemory
    );
    
    // Transition image layout to receive texture
    transitionTextureImageLayout(
        core,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal
    );
    // Copy texture from staging buffer to image
    core.copyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(textureWidth), static_cast<uint32_t>(textureHeight));

    generateMipmaps(core, vk::Format::eR8G8B8A8Srgb);
}

void Texture::createTextureImageView(
    Core const & core
) {
    imageView = core.createImageView(image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, mipLevels);
}

void Texture::load(Core const & core, char const * const texturePath) {
    std::cout << "Loading texture" << std::endl;
    // texturePath.c_str()
    stbi_uc * pixels = stbi_load(texturePath, &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("Failed to load texture.");
    }
    std::cout << "textureWidth = " << textureWidth
    << ", textureHeight = " << textureHeight
    << ", textureChannels = " << textureChannels
    << std::endl;

    computeMipLevels(textureWidth, textureHeight);

    // copy pixels data to texture image
    // texture resources
    createTextureImage(core, pixels);

    // cleanup
    stbi_image_free(pixels);

    // depends on textureImage and mipLevels
    createTextureImageView(core);
}