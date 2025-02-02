/*****************************************************************//**
 * \file   UniformData.cpp
 * \author Adi (aditya.prakash@digipen.edu) and evan.gray (evan.gray@digipen.edu)
 * \date   October 27 2024
 * \Copyright @ 2024 Digipen (USA) Corporation * 
 
 * \brief  Implementations for all uniform init functions
 *  *********************************************************************/
#include <PCH/pch.h>

#include "UniformData.h"
#include "Uniform.h"

#include "../Texture/TextureLibrary.h"
#include "../Font/FontLibrary.h"
#include "../Models/ModelLibrary.h"
#include "../Core/SwapChain.h"

namespace Dog
{
  void MatrixUniformInit(Uniform& uniform, TextureLibrary& textureLibrary, FontLibrary& fontLibrary, ModelLibrary& modelLibrary)
  {
    uniform.GetDescriptorSets().resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    // Build descriptor sets for each frame with both buffer and texture data
    for (int frameIndex = 0; frameIndex < SwapChain::MAX_FRAMES_IN_FLIGHT; ++frameIndex) {
      DescriptorWriter writer(*uniform.GetDescriptorLayout(), *uniform.GetDescriptorPool());

      // Bind the uniform buffer at binding 0
      VkDescriptorBufferInfo bufferInfo = uniform.GetUniformBuffer(0, frameIndex)->descriptorInfo();
      writer.WriteBuffer(0, &bufferInfo);

      // Build descriptor set for the frame
      writer.Build(uniform.GetDescriptorSets()[frameIndex]);
    }
  }

  void InstancedUniformInit(Uniform& uniform, TextureLibrary& textureLibrary, FontLibrary& fontLibrary, ModelLibrary& modelLibrary)
  {
    uniform.GetDescriptorSets().resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    for (int frameIndex = 0; frameIndex < SwapChain::MAX_FRAMES_IN_FLIGHT; ++frameIndex) 
    {
      DescriptorWriter writer(*uniform.GetDescriptorLayout(), *uniform.GetDescriptorPool());

      VkDescriptorBufferInfo bufferInfo0 = uniform.GetUniformBuffer(0, frameIndex)->descriptorInfo();
      VkDescriptorBufferInfo bufferInfo1 = uniform.GetUniformBuffer(1, frameIndex)->descriptorInfo();
      
      writer.WriteBuffer(0, &bufferInfo0);
      writer.WriteBuffer(1, &bufferInfo1);

      writer.Build(uniform.GetDescriptorSets()[frameIndex]);
    }
  }

  void SharedUniformInit(Uniform& uniform, TextureLibrary& textureLibrary, FontLibrary& fontLibrary, ModelLibrary& modelLibrary)
  {
    const Font& fontData = fontLibrary.GetFontByIndex(0);
    const GlyphDataBuffer& glyphBuffer = fontData.GetGlyphBuffer();

    std::vector<VkDescriptorImageInfo> imageInfos(TextureLibrary::MAX_TEXTURE_COUNT);

    VkSampler defaultSampler = textureLibrary.GetTextureSampler();
    size_t textureCount = textureLibrary.getTextureCount();

    for (size_t j = 0; j < TextureLibrary::MAX_TEXTURE_COUNT; ++j) {
      imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfos[j].sampler = defaultSampler;
      imageInfos[j].imageView = textureLibrary.getTextureByIndex(std::min(j, textureCount - 1)).getImageView();
    }

    uniform.GetDescriptorSets().resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    for (int frameIndex = 0; frameIndex < SwapChain::MAX_FRAMES_IN_FLIGHT; ++frameIndex)
    {
      DescriptorWriter writer(*uniform.GetDescriptorLayout(), *uniform.GetDescriptorPool());

      VkDescriptorBufferInfo bufferInfo0 = uniform.GetUniformBuffer(0, frameIndex)->descriptorInfo();
      VkDescriptorBufferInfo bufferInfo2 = uniform.GetUniformBuffer(2, frameIndex)->descriptorInfo();

      writer.WriteBuffer(0, &bufferInfo0);
      writer.WriteImage(1, imageInfos.data(), static_cast<uint32_t>(imageInfos.size()));
      writer.WriteBuffer(2, &bufferInfo2);

      // Build descriptor set for the frame
      writer.Build(uniform.GetDescriptorSets()[frameIndex]);

      uniform.SetUniformData(glyphBuffer.data, 0, frameIndex);
    }
  }

  void ComputeUniformInit(Uniform& uniform, TextureLibrary& textureLibrary, FontLibrary& fontLibrary, ModelLibrary& modelLibrary)
  {
    uniform.GetDescriptorSets().resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    for (int frameIndex = 0; frameIndex < SwapChain::MAX_FRAMES_IN_FLIGHT; ++frameIndex)
    {
      DescriptorWriter writer(*uniform.GetDescriptorLayout(), *uniform.GetDescriptorPool());

      VkDescriptorBufferInfo bufferInfo0 = uniform.GetUniformBuffer(0, frameIndex)->descriptorInfo();
      VkDescriptorBufferInfo bufferInfo1 = uniform.GetUniformBuffer(1, frameIndex)->descriptorInfo();
      VkDescriptorBufferInfo bufferInfo2 = uniform.GetUniformBuffer(2, frameIndex)->descriptorInfo();
      
      writer.WriteBuffer(0, &bufferInfo0);
      writer.WriteBuffer(1, &bufferInfo1);
      writer.WriteBuffer(2, &bufferInfo2);
      
      // Build descriptor set for the frame
      writer.Build(uniform.GetDescriptorSets()[frameIndex]);
    }
  }
  
  void HeightmapUniformInit(Uniform& uniform, TextureLibrary& textureLibrary, FontLibrary& fontLibrary, ModelLibrary& modelLibrary)
  {
    uniform.GetDescriptorSets().resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    // Build descriptor sets for each frame
    for (int frameIndex = 0; frameIndex < SwapChain::MAX_FRAMES_IN_FLIGHT; ++frameIndex) {
      DescriptorWriter writer(*uniform.GetDescriptorLayout(), *uniform.GetDescriptorPool());

      // Bind the uniform buffer at binding 0
      VkDescriptorBufferInfo bufferInfo = uniform.GetUniformBuffer(0, frameIndex)->descriptorInfo();
      writer.WriteBuffer(0, &bufferInfo);

      // Build descriptor set for the frame
      writer.Build(uniform.GetDescriptorSets()[frameIndex]);
    }
  }
}
