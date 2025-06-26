#pragma once

#include "Texture.h"

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
		const std::unique_ptr<Texture>& GetTexture(uint32_t index) const;
		VkSampler GetTextureSampler() { return textureSampler; }

		Texture& getTextureByIndex(const size_t& index) { 
			if (index == INVALID_TEXTURE_INDEX) {

			}
			return *textures[index];
		}

		void CreateTextureSampler();
		void CreateHtmlTextureSampler();

		void CreateDescriptorSet(Texture& texture, bool html = false);
		void UpdateDescriptorSet(Texture& texture, bool html = false);
		VkDescriptorSet GetDescriptorSet(Texture& texture) { return texture.descriptorSet; }
		VkDescriptorSet GetDescriptorSet(const std::string& texturePath) { return GetTexture(GetTexture(texturePath))->descriptorSet; }

		const size_t getTextureCount() const { return textures.size(); }

		void OnTextureFileCreate(const Event::TextureFileCreated& event);
        void OnTextureFileDelete(const Event::TextureFileDeleted& event);
        void OnTextureFileModify(const Event::TextureFileModified& event);
		void UpdateTextures();

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

		uint32_t bakedInTextureCount = 0;

		VkSampler textureSampler;
		VkSampler htmlTextureSampler;

		VkDescriptorPool mTexturePool;
		VkDescriptorSetLayout mTextureDescriptorLayout;
		VkDescriptorSet mOffscreenDS;

		Events::Handle<Event::TextureFileCreated> textureFileCreatedHandle;
		Events::Handle<Event::TextureFileDeleted> textureFileDeletedHandle;
		Events::Handle<Event::TextureFileModified> textureFileModifiedHandle;

        std::vector<std::string> mTexturesToAdd;
        std::vector<std::string> mTexturesToDelete;
        std::vector<std::string> mTexturesToModify;
	};

} // namespace Dog