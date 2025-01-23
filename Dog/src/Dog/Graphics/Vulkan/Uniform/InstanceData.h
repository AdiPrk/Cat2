/*****************************************************************//**
 * \file   InstancingResource.hpp
 * \author Adi (aditya.prakash@digipen.edu)
 * \date   October 25 2024
 * \Copyright @ 2024 Digipen (USA) Corporation *

 * \brief  Holds all the data to be renderered!
 *  *********************************************************************/

#pragma once

namespace Dog
{
    struct alignas(16) InstancedUniformData
    {
        InstancedUniformData()
            : modelMatrix(1.0f)
            , normalMatrix(1.0f)
            , boundingSphere(1.0f)
            , textureIndex(0)
            , charIndex(0)
            , animationIndex(0)
            , meshIndex(0)
            , followCamera(1)
        {
        }

        glm::mat4 modelMatrix;    // 64 bytes
        glm::mat4 normalMatrix;   // 64 bytes
        glm::vec4 boundingSphere; // 16 bytes
        uint32_t textureIndex;    // 4 bytes
        uint32_t charIndex;       // 4 bytes 
        uint32_t animationIndex;  // 4 bytes
        uint32_t meshIndex;       // 4 bytes
        uint32_t followCamera;      // 4 bytes

        // 4 bytes padding
        // 4 bytes padding
        // 4 bytes padding

// there's 12 extra bytes of padding - can probably condense some of these indices

        static const int MAX_INSTANCE_COUNT = 1000000;
    };

    struct MeshData
    {
        uint32_t indexCount; // 4 bytes
        uint32_t firstIndex; // 4 bytes
        int vertexOffset;    // 4 bytes
        // no additional padding needed

        MeshData(uint32_t indexCount, uint32_t firstIndex, int vertexOffset)
            : indexCount(indexCount)
            , firstIndex(firstIndex)
            , vertexOffset(vertexOffset)
        {
        }
    };

    // std::vector<InstancedUniformData> instanceData;
}
