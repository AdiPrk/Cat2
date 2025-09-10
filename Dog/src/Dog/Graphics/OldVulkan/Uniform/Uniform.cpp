/*****************************************************************//**
 * \file   Uniform.cpp
 * \author Adi (aditya.prakash@digipen.edu) and evan.gray (evan.gray@digipen.edu)
 * \date   October 27 2024
 * \Copyright @ 2024 Digipen (USA) Corporation * 
 
 * \brief  Implementation of Uniform
 *  *********************************************************************/
#include <PCH/pch.h>

#include "Uniform.h"
#include "../Core/SwapChain.h"
#include "../Texture/TextureLibrary.h"

namespace Dog
{
  Uniform::Uniform(Device& device, TextureLibrary& textureLibrary, FontLibrary& fontLibrary, ModelLibrary& modelLibrary, const UniformSettings& settings) 
  {
    DescriptorPool::Builder poolBuilder = DescriptorPool::Builder(device).SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (const auto& bindingInfo : settings.bindings) 
    {
      poolBuilder.AddPoolSize(
        bindingInfo.layoutBinding.descriptorType, 
        SwapChain::MAX_FRAMES_IN_FLIGHT * bindingInfo.layoutBinding.descriptorCount
      );
    }
    mUniformPool = poolBuilder.Build();

    for (const auto& bindingInfo : settings.bindings) 
    {
      std::vector<std::unique_ptr<Buffer>> buffers;

      for (int frameIndex = 0; frameIndex < SwapChain::MAX_FRAMES_IN_FLIGHT; ++frameIndex) 
      {
        auto buffer = std::make_unique<Buffer>(
          device,
          bindingInfo.elementSize,
          static_cast<uint32_t>(bindingInfo.elementCount),
          bindingInfo.bufferUsage,
          VMA_MEMORY_USAGE_CPU_TO_GPU
        );
        buffer->map();

        buffers.push_back(std::move(buffer));
      }

      mBuffersPerBinding[bindingInfo.layoutBinding.binding] = std::move(buffers);
    }

    DescriptorSetLayout::Builder layoutBuilder(device);
    for (const auto& bindingInfo : settings.bindings) 
    {
      layoutBuilder.AddBinding(bindingInfo.layoutBinding);
    }
    mUniformDescriptorLayout = layoutBuilder.Build();

    settings.Init(*this, textureLibrary, fontLibrary, modelLibrary);
  }

  void Uniform::Bind(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, int frameIndex, VkPipelineBindPoint bindPoint)
  {
    vkCmdBindDescriptorSets(
      commandBuffer, 
      bindPoint, 
      pipelineLayout, 
      mPipelineBindingIndex, 
      1, 
      &mUniformDescriptorSets[frameIndex],
      0, 
      nullptr);
  }

}
