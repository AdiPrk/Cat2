#pragma once

namespace Dog
{
	class Device;

	class Texture
	{
	public:

		Texture(Device& device, const std::string& path, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB);
		Texture(Device& device, const std::string& name, const unsigned char* textureData, uint32_t textureSize, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB);

		~Texture();

		const int GetWidth() const { return mTexWidth; }
		const int GetHeight() const { return mTexHeight; }
		const VkImageView& GetImageView() const { return mTextureImageView; }
		static void TransitionImageLayout(Device& mDevice, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

	private:

		void LoadPixelsFromFile(const std::string& filepath);
		void LoadPixelsFromMemory(const unsigned char* textureData, int textureSize);

		void CreateTextureImage();

		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage);

		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

		void GenerateMipmaps();

		void CreateTextureImageView();

		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

		Device& mDevice;
		std::string mPath;

		VkImage mTextureImage;
		VmaAllocation mTextureImageAllocation;
		VkImageView mTextureImageView;

		uint32_t mMipLevels;
		int mTexWidth, mTexHeight, mTexChannels;
		VkDeviceSize mImageSize;
		unsigned char* mPixels;

		VkFormat mImageFormat;
	};
}
