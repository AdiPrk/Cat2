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
		, bakedInTextureCount(0)
	{
		CreateTextureSampler();
		CreateHtmlTextureSampler();

		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize.descriptorCount = MAX_TEXTURE_COUNT; // Number of textures

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = MAX_TEXTURE_COUNT;

		vkCreateDescriptorPool(device, &poolInfo, nullptr, &mTexturePool);

		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = 0; // Binding index in the shader
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBinding.descriptorCount = 1; // Each descriptor represents one texture
		layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // Used in fragment shaders
		layoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &layoutBinding;

		vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &mTextureDescriptorLayout);

		textureFileCreatedHandle = SUBSCRIBE_EVENT(Event::TextureFileCreated, OnTextureFileCreate);
		textureFileDeletedHandle = SUBSCRIBE_EVENT(Event::TextureFileDeleted, OnTextureFileDelete);
		textureFileModifiedHandle = SUBSCRIBE_EVENT(Event::TextureFileModified, OnTextureFileModify);
	}

	TextureLibrary::~TextureLibrary()
	{
		vkDestroySampler(device, textureSampler, nullptr);
        vkDestroyDescriptorPool(device, mTexturePool, nullptr);
        vkDestroyDescriptorSetLayout(device, mTextureDescriptorLayout, nullptr);
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

            CreateDescriptorSet(*textures.back());

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

        CreateDescriptorSet(*textures.back());

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

	const std::unique_ptr<Texture>& TextureLibrary::GetTexture(uint32_t index) const
	{
        if (index > textures.size()) {
            // throw std::runtime_error("Texture index out of bounds");
            DOG_WARN("Texture {0} not found", index);
            return textures[0];
        }

        return textures[index];
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

	void TextureLibrary::CreateHtmlTextureSampler() {
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

		// Force Linear filtering (should blur text)
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		// Use extreme addressing modes (should create visual artifacts at edges)
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;

		// Enable max anisotropy (should introduce some stretching)
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

		// Use mipmapping with an extreme bias (should distort the texture)
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 5.0f; // High bias to force using lower mip levels
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = properties.limits.maxSamplerLodBias;

		// Border color (not important for repeat mode)
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;

		// Normalized UVs
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		if (vkCreateSampler(device, &samplerInfo, nullptr, &htmlTextureSampler) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create HTML texture sampler!");
		}
	}

	void TextureLibrary::CreateDescriptorSet(Texture& texture, bool html)
	{
        if (texture.descriptorSet != VK_NULL_HANDLE) {
            return; // Descriptor set already created
        }

		auto& descSet = texture.descriptorSet;
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mTexturePool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &mTextureDescriptorLayout; // Use the layout created above

		vkAllocateDescriptorSets(device, &allocInfo, &descSet);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = texture.getImageView(); // Get the VkImageView from the texture
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        
		if (html) imageInfo.sampler = htmlTextureSampler;
        else imageInfo.sampler = textureSampler;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descSet;
		descriptorWrite.dstBinding = 0; // Binding in the shader
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
	}

	void TextureLibrary::UpdateDescriptorSet(Texture& texture, bool html)
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = texture.getImageView(); // New image view
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        if (html) imageInfo.sampler = htmlTextureSampler;
        else imageInfo.sampler = textureSampler;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = texture.descriptorSet; // Use existing descriptor set handle
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
	}

	void TextureLibrary::OnTextureFileCreate(const Event::TextureFileCreated& event)
	{
        // DOG_INFO("Texture file created: {0}", event.path);
        mTexturesToAdd.push_back(event.path);
	}

	void TextureLibrary::OnTextureFileDelete(const Event::TextureFileDeleted& event)
	{
        // DOG_INFO("Texture file deleted: {0}", event.path);
        mTexturesToDelete.push_back(event.path);
	}

	void TextureLibrary::OnTextureFileModify(const Event::TextureFileModified& event)
	{
        DOG_INFO("Texture file modified: {0}", event.path);
        mTexturesToModify.push_back(event.path);
	}

	void TextureLibrary::UpdateTextures()
	{
        for (const auto& path : mTexturesToAdd) {
            AddTexture(path);
        }

        // for (const auto& path : mTexturesToDelete) 
		// {
        //     uint32_t index = GetTexture(path);
        //     auto& texture = GetTexture(index);
        //     textureMap.erase(path);
        //     textures.erase(textures.begin() + index);
        // }

        mTexturesToAdd.clear();
        mTexturesToDelete.clear();
        mTexturesToModify.clear();

		// Update the uniform
	}

} // namespace Dog