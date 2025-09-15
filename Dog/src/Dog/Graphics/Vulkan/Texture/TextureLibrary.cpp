#include <PCH/pch.h>
#include "TextureLibrary.h"
#include "Texture.h"

#include "../Core/Device.h"

namespace Dog
{
    const uint32_t TextureLibrary::MAX_TEXTURE_COUNT = 10;

    TextureLibrary::TextureLibrary(Device& device)
        : device{ device }
    {
        CreateTextureSampler();
    }

    TextureLibrary::~TextureLibrary()
    {
        vkDestroySampler(device.getDevice(), mTextureSampler, nullptr);
        mTextures.clear();
    }

    uint32_t TextureLibrary::AddTexture(const std::string& filePath)
    {
        // Check if texture already exists
        auto it = mTextureMap.find(filePath);
        if (it != mTextureMap.end()) {
            return it->second; // Return existing texture index
        }

        // Load new texture
        auto newTexture = std::make_unique<Texture>(device, filePath);

        uint32_t newIndex = static_cast<uint32_t>(mTextures.size());
        mTextures.push_back(std::move(newTexture));
        mTextureMap[filePath] = newIndex;
        
        return newIndex;
    }

    Texture* TextureLibrary::GetTexture(uint32_t index)
    {
        if (index < mTextures.size()) {
            return mTextures[index].get();
        }
        return nullptr;
    }

    Texture* TextureLibrary::GetTexture(const std::string& filePath)
    {
        auto it = mTextureMap.find(filePath);
        if (it != mTextureMap.end()) {
            return GetTexture(it->second);
        }
        return nullptr;
    }

    const Texture& TextureLibrary::GetTextureByIndex(uint32_t index) const
    {
        if (index < mTextures.size()) {
            return *mTextures[index];
        }

        DOG_ERROR("Texture index out of range: {0}", index);
        return *mTextures[0];
    }

    void TextureLibrary::CreateTextureSampler()
    {
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

        if (vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &mTextureSampler) != VK_SUCCESS)
        {
            DOG_CRITICAL("Failed to create texture sampler");
        }
    }
}
