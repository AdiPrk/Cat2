/*********************************************************************
 * file:   Uniform.hpp
 * author: aditya.prakash (aditya.prakash@digipen.edu) and evan.gray (evan.gray@digipen.edu)
 * date:   September 24, 2024
 * Copyright © 2024 DigiPen (USA) Corporation. 
 * 
 * brief:  Handles the discriptor sets and buffers for sending
 *         uniform data to shaders
 *********************************************************************/
#pragma once

#include "../Buffers/Buffer.h"
#include "../Descriptors/Descriptors.h"
#include "UniformData.h"

namespace Dog
{
  //Forward declarations
  class TextureLibrary;
  class FontLibrary;
  class ModelLibrary;
  struct UniformSettings;
  struct MatrixUniformData;
  struct GlyphData;

  class Uniform {
  public:
    /*********************************************************************
     * param:  device: The vulkan device
     * param:  textureLibrary: The texture library
     * param:  fontLibrary: The font library
     * param:  modelLibrary: The model library
     * param:  settings: The settings for the uniform
     * 
     * brief:  Constructor for the uniform
     *********************************************************************/
    Uniform(Device& device, TextureLibrary& textureLibrary, FontLibrary& fontLibrary, ModelLibrary& modelLibrary, const UniformSettings& settings);
    ~Uniform() {}

    /*********************************************************************
     * param:  commandBuffer: The command buffer
     * param:  pipelineLayout: The pipeline layout
     * param:  frameIndex: The frame index
     * param:  bindPoint: The pipeline stage to bind to
     * 
     * brief:  Binds the uniform to the command buffer
     *********************************************************************/
    void Bind(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, int frameIndex, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

    /*********************************************************************
     * brief:  Sets the uniform's data
     *********************************************************************/
    template<typename T>
    void SetUniformData(const T& data, int bindingIndex, int frameIndex);

    template<typename T>
    void SetUniformData(const std::vector<T>& data, int bindingIndex, int frameIndex);

    template<typename T, std::size_t N>
    void SetUniformData(const std::array<T, N>& data, int bindingIndex, int frameIndex);

    /*********************************************************************
     * param:  binding: The descriptor set's binding index
     * 
     * brief:  Sets the descriptor set's binding index
     *********************************************************************/
    void SetBinding(unsigned int binding) { mPipelineBindingIndex = binding; }

    // Getters
    std::unique_ptr<DescriptorSetLayout>& GetDescriptorLayout() { return mUniformDescriptorLayout; }
    std::vector<VkDescriptorSet>& GetDescriptorSets() { return mUniformDescriptorSets; }
    std::unique_ptr<Buffer>& GetUniformBuffer(int binding, int frameIndex) { return mBuffersPerBinding[binding][frameIndex]; }
    std::unique_ptr<DescriptorPool>& GetDescriptorPool() { return mUniformPool; }

  private:
    std::unordered_map<int, std::vector<std::unique_ptr<Buffer>>> mBuffersPerBinding;
    std::vector<VkDescriptorSet> mUniformDescriptorSets;
    std::unique_ptr<DescriptorPool> mUniformPool;
    std::unique_ptr<DescriptorSetLayout> mUniformDescriptorLayout;
    unsigned int mPipelineBindingIndex = std::numeric_limits<unsigned int>::max();
  };

} // namespace Rendering

#include "Uniform.inl"