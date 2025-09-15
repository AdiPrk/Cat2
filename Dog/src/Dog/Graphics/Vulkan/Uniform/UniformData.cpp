#include <PCH/pch.h>

#include "UniformData.h"
#include "Uniform.h"
#include "Descriptors.h"

#include "ECS/Resources/RenderingResource.h"
#include "../Core/Buffer.h"
#include "../Core/SwapChain.h"

#include "../Texture/Texture.h"

namespace Dog
{
    void CameraUniformInit(Uniform& uniform, RenderingResource& renderData)
    {
        uniform.GetDescriptorSets().resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        // Build descriptor sets for each frame with both buffer and texture data
        for (int frameIndex = 0; frameIndex < SwapChain::MAX_FRAMES_IN_FLIGHT; ++frameIndex) {
            DescriptorWriter writer(*uniform.GetDescriptorLayout(), *uniform.GetDescriptorPool());

            // Bind the uniform buffer at binding 0
            VkDescriptorBufferInfo bufferInfo = uniform.GetUniformBuffer(0, frameIndex)->DescriptorInfo();
            writer.WriteBuffer(0, &bufferInfo);

            // Build descriptor set for the frame
            writer.Build(uniform.GetDescriptorSets()[frameIndex]);
        }
    }

    void InstanceUniformInit(Uniform& uniform, RenderingResource& renderData)
    {
        uniform.GetDescriptorSets().resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        std::vector<VkDescriptorImageInfo> imageInfos(TextureLibrary::MAX_TEXTURE_COUNT);

        VkSampler defaultSampler = renderData.textureLibrary->GetSampler();
        size_t textureCount = renderData.textureLibrary->GetTextureCount();

        for (size_t j = 0; j < TextureLibrary::MAX_TEXTURE_COUNT; ++j) {
            imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[j].sampler = defaultSampler;
            imageInfos[j].imageView = renderData.textureLibrary->GetTextureByIndex(std::min(j, textureCount - 1)).GetImageView();
        }

        for (int frameIndex = 0; frameIndex < SwapChain::MAX_FRAMES_IN_FLIGHT; ++frameIndex) 
        {
            DescriptorWriter writer(*uniform.GetDescriptorLayout(), *uniform.GetDescriptorPool());

            VkDescriptorBufferInfo bufferInfo = uniform.GetUniformBuffer(1, frameIndex)->DescriptorInfo();

            writer.WriteImage(0, imageInfos.data(), static_cast<uint32_t>(imageInfos.size()));
            writer.WriteBuffer(1, &bufferInfo);

            writer.Build(uniform.GetDescriptorSets()[frameIndex]);
        }
    }
}