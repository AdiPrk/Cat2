/*****************************************************************//**
 * \file   Uniform.inl
 * \author Adi (aditya.prakash@digipen.edu)
 * \date   October 27 2024
 * \Copyright @ 2024 Digipen (USA) Corporation * 
 
 * \brief  Uniform template definitions
 *  *********************************************************************/

#pragma once
#include "Uniform.h"

namespace Dog {

  //if constexpr (std::is_same_v<T, VkDescriptorImageInfo>) {
  //
  //}
  //else {
  //}

  template<typename T>
  void Uniform::SetUniformData(const T& data, int bindingIndex, int frameIndex) 
  {
    auto& buffer = mBuffersPerBinding[bindingIndex][frameIndex];
    VkDeviceSize dataSize = sizeof(T);

    buffer->writeToBuffer(&data, dataSize);
  }

  template<typename T>
  void Uniform::SetUniformData(const std::vector<T>& data, int bindingIndex, int frameIndex) 
  {
    auto& buffer = mBuffersPerBinding[bindingIndex][frameIndex];
    VkDeviceSize dataSize = data.size() * sizeof(T);
    
    buffer->writeToBuffer(data.data(), dataSize);
  }

  template<typename T, std::size_t N>
  void Uniform::SetUniformData(const std::array<T, N>& data, int bindingIndex, int frameIndex)
  {
    auto& buffer = mBuffersPerBinding[bindingIndex][frameIndex];
    VkDeviceSize dataSize = N * sizeof(T);

    buffer->writeToBuffer(data.data(), dataSize);
  }

} // namespace Rendering
