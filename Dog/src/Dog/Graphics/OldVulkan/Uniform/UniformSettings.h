/*****************************************************************//**
 * \file   UniformSettings.hpp
 * \author Adi (aditya.prakash@digipen.edu)
 * \date   October 27 2024
 * \Copyright @ 2024 Digipen (USA) Corporation * 
 
 * \brief  Uniform settings!
 *  *********************************************************************/

#pragma once

namespace Dog
{
  class Uniform;
  class TextureLibrary;
  class FontLibrary;
  class ModelLibrary;

  struct UniformBindingInfo 
  {
    VkDescriptorSetLayoutBinding layoutBinding;  // Layout info for this binding
    VkBufferUsageFlags bufferUsage;              // Buffer usage (e.g., VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
    size_t elementSize;                          // Size of each element in this binding
    size_t elementCount;                         // Number of elements for dynamic buffers (SSBOs)
  };

  struct UniformSettings 
  {
    std::vector<UniformBindingInfo> bindings;  // List of binding configurations
    void (*Init)(Uniform&, TextureLibrary&, FontLibrary&, ModelLibrary&);
    uint32_t nextBinding = 0;  // Tracks the next available binding slot

    explicit UniformSettings(void (*initFunc)(Uniform&, TextureLibrary&, FontLibrary&, ModelLibrary&))
      : Init(initFunc) 
    {}

    // Add a uniform buffer binding
    UniformSettings& AddUBBinding(VkShaderStageFlags stageFlags, size_t elementSize, size_t elementCount = 1)
    {
      bindings.push_back({ { nextBinding++, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, stageFlags }, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, elementSize, elementCount });
      return *this;
    }

    // Add a storage buffer binding
    UniformSettings& AddSSBOBinding(VkShaderStageFlags stageFlags, size_t elementSize, size_t elementCount) 
    {
      bindings.push_back({ { nextBinding++, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, stageFlags }, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, elementSize, elementCount });
      return *this;
    }

    UniformSettings& AddSSBOIndirectBinding(VkShaderStageFlags stageFlags, size_t elementSize, size_t elementCount)
    {
      bindings.push_back({ { nextBinding++, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, stageFlags }, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |    VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, elementSize, elementCount });
      return *this;
    }

    // Add an image sampler binding
    UniformSettings& AddISBinding(VkShaderStageFlags stageFlags, size_t elementSize, uint32_t descriptorCount) 
    {
      bindings.push_back({ { nextBinding++, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorCount, stageFlags }, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, elementSize, descriptorCount });
      return *this;
    }
  };
}
