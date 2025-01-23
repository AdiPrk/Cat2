#include <PCH/pch.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <vulkan/vulkan.h>
#include "Texture.h"
#include "../Core/Device.h"

namespace Dog {

    // Constructor
    Texture::Texture(Device& device, const std::string& filepath)
        : device{ device }
    {
        path = filepath;

        createTextureImage(filepath);
        createTextureImageView();
    }

    Texture::Texture(Device& device, const std::string& filepath, const unsigned char* textureData, int textureSize)
        : device{ device }
    {
        path = filepath;

        createTextureImageFromMemory(textureData, textureSize);
        createTextureImageView();
    }

    // Destructor
    Texture::~Texture() {
        vkDestroyImageView(device, textureImageView, nullptr);
        vmaDestroyImage(device.allocator, textureImage, textureImageAllocation);
    }

    // Load the texture image from file using stb_image
    void Texture::createTextureImage(const std::string& filepath) {
        // Check if file exists
        std::ifstream file(filepath);
        if (!file.good()) {
            throw std::runtime_error("Failed to load texture image file at file path: " + filepath);
        }

        // Flip the image vertically because Vulkan has a different coordinate system
        stbi_set_flip_vertically_on_load(true);

        // Load in the pixel data
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        // This threw needs to be replaced with NLE Critical (or Error)?
        // Maybe add better error handling for textures, so it simply returns the INVALID_TEXTURE texture
        if (!pixels) {
            throw std::runtime_error("Failed to load texture image!");
        }

        // width * height * 4 (rgba)
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        // Mip levels is the number of times the image can be halved in size
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

        // Create a staging buffer to load texture data
        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAllocation;  // VMA allocation replaces VkDeviceMemory
        device.createBuffer(
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_CPU_ONLY,  // Use CPU_ONLY for host-visible memory
            stagingBuffer,
            stagingBufferAllocation
        );

        // Copy pixel data to staging buffer
        void* data;
        vmaMapMemory(device.allocator, stagingBufferAllocation, &data);  // VMA maps memory for you
        memcpy(data, pixels, static_cast<size_t>(imageSize));  // Copy pixel data into mapped memory
        vmaUnmapMemory(device.allocator, stagingBufferAllocation);  // Unmap the memory after copy

        // Free the pixel data
        stbi_image_free(pixels);

        // Create the Vulkan image
        createImage(texWidth, texHeight, mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VMA_MEMORY_USAGE_GPU_ONLY);

        // Transition the image layout and copy from staging buffer
        transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
        copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);

        // Clean up the staging buffer
        vmaDestroyBuffer(device.allocator, stagingBuffer, stagingBufferAllocation);

        GenerateMipmaps(texWidth, texHeight);
    }

    // Load the texture image from file using stb_image
    void Texture::createTextureImageFromMemory(const unsigned char* textureData, int textureSize) {
        // Check if file exists

        stbi_set_flip_vertically_on_load(true);

        int texWidth, texHeight, texChannels;
        //stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        stbi_uc* pixels = stbi_load_from_memory(textureData, textureSize, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("Failed to load texture image!");
        }

        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

        // Create a staging buffer to load texture data
        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAllocation;  // VMA allocation replaces VkDeviceMemory
        device.createBuffer(
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_CPU_ONLY,  // Use CPU_ONLY for host-visible memory
            stagingBuffer,
            stagingBufferAllocation
        );

        // Copy pixel data to staging buffer
        void* data;
        vmaMapMemory(device.allocator, stagingBufferAllocation, &data);  // VMA maps memory for you
        memcpy(data, pixels, static_cast<size_t>(imageSize));  // Copy pixel data into mapped memory
        vmaUnmapMemory(device.allocator, stagingBufferAllocation);  // Unmap the memory after copy

        // Free the pixel data
        stbi_image_free(pixels);

        // Create the Vulkan image
        createImage(texWidth,
            texHeight,
            mipLevels,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VMA_MEMORY_USAGE_GPU_ONLY);

        // Transition the image layout and copy from staging buffer
        transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
        copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);

        // Clean up the staging buffer
        vmaDestroyBuffer(device.allocator, stagingBuffer, stagingBufferAllocation);

        // Generate mipmaps for the texture
        GenerateMipmaps(texWidth, texHeight);
    }

    // Create an image view for the texture
    void Texture::createTextureImageView() {
        textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
    }

    void Texture::GenerateMipmaps(int32_t texWidth, int32_t texHeight)
    {
        // Check if image format supports linear blitting
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(device.getPhysicalDevice(), VK_FORMAT_R8G8B8A8_SRGB, &formatProperties);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = textureImage;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = texWidth;
        int32_t mipHeight = texHeight;

        for (uint32_t i = 1; i < mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(commandBuffer,
                textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        device.endSingleTimeCommands(commandBuffer);
    }

    void Texture::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage) {
        // The parameters for the image creation
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO; // Type of structure
        imageInfo.imageType = VK_IMAGE_TYPE_2D;                // Type of image, could be 1D and 3D too
        imageInfo.extent.width = width;  // Width of the image
        imageInfo.extent.height = height;// Height of the image
        imageInfo.extent.depth = 1;      // Depth of the image (how many texels on the z axis, which is 1 for 2D)
        imageInfo.mipLevels = mipLevels; // Number of mip levels
        imageInfo.arrayLayers = 1;	     // Number of layers in image array (?)
        imageInfo.format = format;       // Format of the image data
        imageInfo.tiling = tiling;       // How texels are laid out in memory
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Layout of the image data on creation
        imageInfo.usage = usage;                             // Bit field of what the image will be used for
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;   // It'll only be accessed by the graphics queue family
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;           // Number of samples for multisampling
        imageInfo.flags = 0; // Optional

        // Setup allocation info for VMA
        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = memoryUsage;

        // Use VMA to create image and allocate memory in one step
        if (vmaCreateImage(device.allocator, &imageInfo, &allocInfo, &textureImage, &textureImageAllocation, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create and allocate image using VMA!");
        }
    }

    VkImageView Texture::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create texture image view!");
        }

        return imageView;
    }

    void Texture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {

        VkCommandBuffer commandBuffer = device.beginSingleTimeCommands(); // Helper to start recording commands

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else {
            throw std::invalid_argument("Unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        device.endSingleTimeCommands(commandBuffer); // Helper to finish recording and submit the commands
    }

    void Texture::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) {
        VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layerCount;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, 1 };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region);
        device.endSingleTimeCommands(commandBuffer);
    }

} // namespace Dog