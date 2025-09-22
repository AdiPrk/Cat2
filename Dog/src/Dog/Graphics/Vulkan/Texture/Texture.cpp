#include <PCH/pch.h>
#include "Texture.h"

#include "../Core/Device.h"
#include "../Core/Buffer.h"

#include "stb_image.h"

namespace Dog
{
	Texture::Texture(Device& device, const std::string& path, VkFormat imageFormat)
		: mDevice(device)
		, mPath(path)
        , mImageFormat(imageFormat)
	{
		if (!std::filesystem::exists(path))
		{
            DOG_CRITICAL("File path {0} doesn't exist!", path);
		}

		LoadPixelsFromFile(path);
		CreateTextureImage();
		CreateTextureImageView();
	}

	Texture::Texture(Device& device, const std::string& name, const unsigned char* textureData, uint32_t textureSize, VkFormat imageFormat)
		: mDevice(device)
		, mPath(name)
        , mImageFormat(imageFormat)
	{
		LoadPixelsFromMemory(textureData, static_cast<int>(textureSize));
		CreateTextureImage();
		CreateTextureImageView();
	}

	Texture::~Texture()
	{
		if (mTextureImageView)
		{
			vkDestroyImageView(mDevice.GetDevice(), mTextureImageView, nullptr);
			mTextureImageView = VK_NULL_HANDLE;
		}

		if (mTextureImage)
		{
			vmaDestroyImage(mDevice.GetVmaAllocator(), mTextureImage, mTextureImageAllocation);
			mTextureImage = VK_NULL_HANDLE;
			mTextureImageAllocation = VK_NULL_HANDLE;
		}
	}

	void Texture::CreateTextureImage()
	{
		// Mip levels is the number of times the image can be halved in size
		mMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(mTexWidth, mTexHeight)))) + 1;

		// Create a staging buffer to load texture data
		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferAllocation;  // VMA allocation replaces VkDeviceMemory
		mDevice.GetAllocator()->CreateBuffer(
			mImageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_ONLY,  // Use CPU_ONLY for host-visible memory
			stagingBuffer,
			stagingBufferAllocation
		);

		// Copy pixel data to staging buffer
		void* data;
		vmaMapMemory(mDevice.GetVmaAllocator(), stagingBufferAllocation, &data);  // VMA maps memory for you
		memcpy(data, mPixels, static_cast<size_t>(mImageSize));  // Copy pixel data into mapped memory
		vmaUnmapMemory(mDevice.GetVmaAllocator(), stagingBufferAllocation);  // Unmap the memory after copy

		// Free the pixel data
		stbi_image_free(mPixels);

		//Set the usage flags for the image
		VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

		// Create the Vulkan image
		CreateImage(mTexWidth, mTexHeight, mImageFormat, VK_IMAGE_TILING_OPTIMAL, usage, VMA_MEMORY_USAGE_GPU_ONLY);

		// Transition the image to a transfer destination layout
		TransitionImageLayout(mDevice, mTextureImage, mImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mMipLevels);

		// Copy the staging buffer to the image
		CopyBufferToImage(stagingBuffer, mTextureImage, static_cast<uint32_t>(mTexWidth), static_cast<uint32_t>(mTexHeight), 1);

		// Destroy the staging buffer
		vmaDestroyBuffer(mDevice.GetVmaAllocator(), stagingBuffer, stagingBufferAllocation);

		// Generate mipmaps
		GenerateMipmaps();
	}

	void Texture::TransitionImageLayout(Device& mDevice, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
	{
		VkCommandBuffer commandBuffer = mDevice.BeginSingleTimeCommands(); // Helper to start recording commands

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

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = 0; // No need to wait for anything in VK_IMAGE_LAYOUT_UNDEFINED
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // Shader will read from the image

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Assuming the image is read in the fragment shader
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) {
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
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

		mDevice.EndSingleTimeCommands(commandBuffer); // Helper to finish recording and submit the commands
	}

	void Texture::LoadPixelsFromFile(const std::string& filepath)
	{
		stbi_set_flip_vertically_on_load(true);

		// Load in the pixel data
		mPixels = stbi_load(filepath.c_str(), &mTexWidth, &mTexHeight, &mTexChannels, STBI_rgb_alpha);

		if (!mPixels) {
            DOG_CRITICAL("stbi_load failed from path: {0}", filepath);
		}

		// width * height * 4 (rgba)
		mImageSize = mTexWidth * mTexHeight * 4;
	}

	void Texture::LoadPixelsFromMemory(const unsigned char* textureData, int textureSize)
	{
		stbi_set_flip_vertically_on_load(true);

		// Load in the pixel data
		mPixels = stbi_load_from_memory(textureData, static_cast<int>(textureSize), &mTexWidth, &mTexHeight, &mTexChannels, STBI_rgb_alpha);

		if (!mPixels) {
            DOG_CRITICAL("stbi_load_from_memory failed");
		}

		// width * height * 4 (rgba)
		mImageSize = mTexWidth * mTexHeight * 4;
	}

	void Texture::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage)
	{
		// The parameters for the image creation
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO; // Type of structure
		imageInfo.imageType = VK_IMAGE_TYPE_2D;                // Type of image, could be 1D and 3D too
		imageInfo.extent.width = width;       // Width of the image
		imageInfo.extent.height = height;     // Height of the image
		imageInfo.extent.depth = 1;           // Depth of the image (how many texels on the z axis, which is 1 for 2D)
		imageInfo.mipLevels = mMipLevels;      // Number of mip levels
		imageInfo.arrayLayers = 1;	          // Number of layers in image array (?)
		imageInfo.format = format;            // Format of the image data
		imageInfo.tiling = tiling;            // How texels are laid out in memory
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Layout of the image data on creation
		imageInfo.usage = usage;                             // Bit field of what the image will be used for
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;   // It'll only be accessed by the graphics queue family
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;           // Number of samples for multisampling
		imageInfo.flags = 0; // Optional

		// Setup allocation info for VMA
		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = memoryUsage;

		// Use VMA to create image and allocate memory in one step
		if (vmaCreateImage(mDevice.GetVmaAllocator(), &imageInfo, &allocInfo, &mTextureImage, &mTextureImageAllocation, nullptr) != VK_SUCCESS)
		{
            DOG_CRITICAL("Failed to create and allocate image using VMA!");
		}
	}

	void Texture::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount)
	{
		VkCommandBuffer commandBuffer = mDevice.BeginSingleTimeCommands();

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

		mDevice.EndSingleTimeCommands(commandBuffer);
	}

	void Texture::GenerateMipmaps()
	{
		// Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(mDevice.GetPhysicalDevice(), mImageFormat, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
            DOG_CRITICAL("texture image format does not support linear blitting!");
		}

		VkCommandBuffer commandBuffer = mDevice.BeginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = mTextureImage;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = mTexWidth;
		int32_t mipHeight = mTexHeight;

		for (uint32_t i = 1; i < mMipLevels; i++) {
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
				mTextureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				mTextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
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

		barrier.subresourceRange.baseMipLevel = mMipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		mDevice.EndSingleTimeCommands(commandBuffer);
	}

	void Texture::CreateTextureImageView() {
		mTextureImageView = CreateImageView(mTextureImage, mImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	VkImageView Texture::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mMipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(mDevice.GetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		{
            DOG_CRITICAL("Failed to create texture image view!");
		}

		return imageView;
	}
}
