#include <PCH/pch.h>
#include "TextureLibrary.h"
#include "Engine.h"

namespace Dog {

	const uint32_t TextureLibrary::INVALID_TEXTURE_INDEX = 9999;
	const uint32_t TextureLibrary::MAX_TEXTURE_COUNT = 10000;
	const uint32_t TextureLibrary::NO_TEXTURE_INDEX = 10001;
	const uint32_t TextureLibrary::WIREFRAME_TEXTURE_INDEX = 10002;

	TextureLibrary::TextureLibrary(Device& device)
		: device(device)
		, imGuiTextureManager(device)
		, bakedInTextureCount(0)
	{
		CreateTextureSampler();
	}

	TextureLibrary::~TextureLibrary()
	{
		vkDestroySampler(device, textureSampler, nullptr);
	}

	uint32_t TextureLibrary::AddTexture(const std::string& texturePath) {
		// check size 
		if (textures.size() >= MAX_TEXTURE_COUNT) {
			throw std::runtime_error("Texture count exceeded maximum");
			return INVALID_TEXTURE_INDEX;
		}

		if (textureMap.find(texturePath) == textureMap.end()) 
		{
			uint32_t textureIndex = static_cast<uint32_t>(textures.size());

			textureMap[texturePath] = textureIndex;
			textures.push_back(std::make_unique<Texture>(device, texturePath));

			imGuiTextureManager.AddTexture(texturePath, textures.back()->getImageView(), textureSampler);

			return textureIndex;
		}
		else {
			return textureMap[texturePath];
		}
	}

	// No need to check if texture already exists in this function, because
	// this will only be getting called by models, and models are unique
	uint32_t TextureLibrary::AddTextureFromMemory(const unsigned char* textureData, int textureSize)
	{
		// check size 
		if (textures.size() >= MAX_TEXTURE_COUNT) {
			throw std::runtime_error("Texture count exceeded maximum");
			return INVALID_TEXTURE_INDEX;
		}

		std::string newPath = "BAKED_IN_" + std::to_string(bakedInTextureCount);
		bakedInTextureCount++;

		uint32_t textureIndex = static_cast<uint32_t>(textures.size());

		textureMap[newPath] = textureIndex;
		textures.push_back(std::make_unique<Texture>(device, newPath, textureData, textureSize));

		imGuiTextureManager.AddTexture(newPath, textures.back()->getImageView(), textureSampler);

		return textureIndex;
	}

	uint32_t TextureLibrary::GetTexture(const std::string& texturePath) {
		if (textureMap.find(texturePath) != textureMap.end()) {
			return textureMap[texturePath];
		}
		else {
			return INVALID_TEXTURE_INDEX; // 999 is the 'no texture' id.
		}
	}

	VkDescriptorSet TextureLibrary::GetDescriptorSet(const std::string& texturePath)
	{
		return imGuiTextureManager.GetDescriptorSet(texturePath);
	}

	VkDescriptorSet TextureLibrary::GetDescriptorSetByIndex(const size_t& index)
	{
		return imGuiTextureManager.GetDescriptorSet(
			getTextureByIndex(index).path
		);
	}

	// Create a sampler for the texture
	void TextureLibrary::CreateTextureSampler() {
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create texture sampler!");
		}
	}

} // namespace Dog