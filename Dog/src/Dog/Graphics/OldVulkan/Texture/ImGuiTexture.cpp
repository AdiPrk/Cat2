#include <PCH/pch.h>
#include "ImGuiTexture.h"
#include "../Core/Device.h"
#include "TextureLibrary.h"
#include "../Renderer.h"
#include "Engine.h"
#include "Graphics/Editor/Editor.h"

namespace Dog {

    ImGuiTextureManager::ImGuiTextureManager(Device& device)
        : device(device)
    {
    }

    ImGuiTextureManager::~ImGuiTextureManager()
    {
    }

    VkDescriptorSet ImGuiTextureManager::CreateDescriptorSet(const VkImageView& imageView, const VkSampler& sampler) 
    {
        VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        return descriptorSet;
    }

    void ImGuiTextureManager::AddTexture(const std::string& texturePath, const VkImageView& imageView, const VkSampler& sampler)
    {
        descriptorMap[texturePath] = CreateDescriptorSet(imageView, sampler);
    }

    VkDescriptorSet ImGuiTextureManager::GetDescriptorSet(const std::string& texturePath)
    {
        // check if in, otherwise return nullptr
        if (descriptorMap.find(texturePath) == descriptorMap.end())
        {
            throw std::runtime_error("Texture not found in descriptor map!");
        }

        return descriptorMap[texturePath];
    }

} // namespace Dog