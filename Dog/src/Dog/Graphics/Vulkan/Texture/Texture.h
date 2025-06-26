#pragma once

namespace Dog {

    class Device;

    class Texture {
    public:
        Texture(Device& device);
        Texture(Device& device, const std::string& filepath);
        Texture(Device& device, const std::string& filepath, const unsigned char* textureData, int textureSize);
        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        const VkImageView& getImageView() const { return textureImageView; }

        std::string path;

        // Returns if new image was created
        //bool CopyBitmapToTexture(ultralight::RefPtr<ultralight::Bitmap> bitmap);

    private:
        friend class TextureLibrary;

        void createTextureImage(const std::string& filepath);
        void createTextureImageFromMemory(const unsigned char* textureData, int textureSize);
        void createTextureImageView();
        void GenerateMipmaps(int32_t texWidth, int32_t texHeight);

        void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage);

        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
        void copyBufferToImage(
            VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);



        Device& device;
        VkImage textureImage;
        VmaAllocation textureImageAllocation;
        VkImageView textureImageView;

        uint32_t mipLevels;

        VkDescriptorSet descriptorSet;

        VkFormat format;
        uint32_t textureWidth = 0;
        uint32_t textureHeight = 0;
    };

} // namespace Dog