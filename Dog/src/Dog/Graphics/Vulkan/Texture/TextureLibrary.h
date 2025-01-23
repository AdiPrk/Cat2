#pragma once

#include "Texture.h"
#include "ImGuiTexture.h"

namespace Dog {

	class Renderer;

	class TextureLibrary {
	public:
		TextureLibrary(Device& device);
		~TextureLibrary();

		TextureLibrary(const TextureLibrary&) = delete;
		TextureLibrary& operator=(const TextureLibrary&) = delete;

		uint32_t AddTexture(const std::string& texturePath);
		uint32_t AddTextureFromMemory(const unsigned char* textureData, int textureSize);

		uint32_t GetTexture(const std::string& texturePath);
		VkSampler GetTextureSampler() { return textureSampler; }

		VkDescriptorSet GetDescriptorSet(const std::string& texturePath);

		Texture& getTextureByIndex(const size_t& index) { 
			if (index == INVALID_TEXTURE_INDEX) {

			}
			return *textures[index];
		}
		VkDescriptorSet GetDescriptorSetByIndex(const size_t& index);

		void CreateTextureSampler();

		const size_t getTextureCount() const { return textures.size(); }

	public:
		// This index renders the error texture
		const static uint32_t INVALID_TEXTURE_INDEX;

		// The maximum amount of textures that can be loaded
		const static uint32_t MAX_TEXTURE_COUNT;

		// The index for the wireframe textures
		const static uint32_t WIREFRAME_TEXTURE_INDEX;

		// This index will use vertex colors instead of textures
		const static uint32_t NO_TEXTURE_INDEX;

	private:
		std::vector<std::unique_ptr<Texture>> textures;
		std::unordered_map<std::string, uint32_t> textureMap;
		Device& device;

		ImGuiTextureManager imGuiTextureManager;

		uint32_t bakedInTextureCount = 0;

		VkSampler textureSampler;
	};

} // namespace Dog