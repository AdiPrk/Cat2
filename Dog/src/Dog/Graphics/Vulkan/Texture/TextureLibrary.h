#pragma once

namespace Dog
{
    class Device;
	class Texture;
	class Uniform;

	class TextureLibrary
	{
	public:
		TextureLibrary(Device& device);
		~TextureLibrary();

		uint32_t AddTexture(const std::string& texturePath);
		uint32_t AddTexture(const unsigned char* textureData, uint32_t textureSize, const std::string& texturePath);
		uint32_t AddPreloadedTexture(const unsigned char* textureData, uint32_t w, uint32_t h, uint32_t ch, const std::string& texturePath);

		Texture* GetTexture(uint32_t textureID);
		Texture* GetTexture(const std::string& texturePath);
		const Texture& GetTextureByIndex(uint32_t index) const;

		uint32_t GetTextureCount() const { return static_cast<uint32_t>(mTextures.size()); }
        VkSampler GetSampler() const { return mTextureSampler; }

		const static uint32_t MAX_TEXTURE_COUNT;

	private:
        void CreateTextureSampler();

		friend Texture;
		std::vector<std::unique_ptr<Texture>> mTextures;
		std::unordered_map<std::string, uint32_t> mTextureMap;

		Device& device;
        VkSampler mTextureSampler;
	};

} // namespace Rendering