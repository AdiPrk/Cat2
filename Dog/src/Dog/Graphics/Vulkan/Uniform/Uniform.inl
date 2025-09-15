/*****************************************************************//**
 * \file   Uniform.inl
 * \author Adi (aditya.prakash@digipen.edu)
 * \date   October 27 2024
 * \Copyright @ 2024 Digipen (USA) Corporation *

 * \brief  Uniform template definitions
 *  *********************************************************************/

#pragma once
#include "Uniform.h"
#include "../Core/Buffer.h"

namespace Dog 
{
    template<typename T>
    void Uniform::SetUniformData(const T& data, int bindingIndex, int frameIndex)
    {
        auto& buffer = mBuffersPerBinding[bindingIndex][frameIndex];
        VkDeviceSize dataSize = sizeof(T);

        buffer->WriteToBuffer(&data, dataSize);
    }

    template<typename T>
    void Uniform::SetUniformData(const std::vector<T>& data, int bindingIndex, int frameIndex)
    {
        auto& buffer = mBuffersPerBinding[bindingIndex][frameIndex];
        VkDeviceSize dataSize = data.size() * sizeof(T);

        buffer->WriteToBuffer(data.data(), dataSize);
    }

    template<typename T>
    void Uniform::SetUniformData(const std::vector<T>& data, int bindingIndex, int frameIndex, int count)
    {
        auto& buffer = mBuffersPerBinding[bindingIndex][frameIndex];
        VkDeviceSize dataSize = count * sizeof(T);

        buffer->WriteToBuffer(data.data(), dataSize);
    }

    template<typename T, std::size_t N>
    void Uniform::SetUniformData(const std::array<T, N>& data, int bindingIndex, int frameIndex)
    {
        auto& buffer = mBuffersPerBinding[bindingIndex][frameIndex];
        VkDeviceSize dataSize = N * sizeof(T);

        buffer->WriteToBuffer(data.data(), dataSize);
    }

} // namespace Rendering
