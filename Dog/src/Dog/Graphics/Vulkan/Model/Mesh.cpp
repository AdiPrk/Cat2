#include <PCH/pch.h>
#include "Mesh.h"

#include "../Core/Device.h"
#include "../Core/Buffer.h"

namespace Dog
{
    int Mesh::uniqueMeshIndex = 0;

    Mesh::Mesh(bool assignID)
        : mMeshID(0)
    {
        if (assignID)
        {
            mMeshID = uniqueMeshIndex++;
        }
    }

    void Mesh::CreateVertexBuffers(Device& device)
    {
        mVertexCount = static_cast<uint32_t>(mVertices.size());
        //assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(mVertices[0]) * mVertexCount;
        uint32_t vertexSize = sizeof(mVertices[0]);

        Buffer stagingBuffer{
            device,
            vertexSize,
            mVertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_CPU_ONLY,
        };

        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer((void*)mVertices.data());

        mVertexBuffer = std::make_unique<Buffer>(
            device,
            vertexSize,
            mVertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        stagingBuffer.CopyBuffer(stagingBuffer.GetBuffer(), mVertexBuffer->GetBuffer(), bufferSize);
    }

    void Mesh::CreateIndexBuffers(Device& device)
    {
        mIndexCount = static_cast<uint32_t>(mIndices.size());
        mHasIndexBuffer = mIndexCount > 0;

        if (!mHasIndexBuffer) {
            return;
        }

        VkDeviceSize bufferSize = sizeof(mIndices[0]) * mIndexCount;
        uint32_t indexSize = sizeof(mIndices[0]);

        Buffer stagingBuffer{
            device,
            indexSize,
            mIndexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_CPU_ONLY,
        };

        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer((void*)mIndices.data());

        mIndexBuffer = std::make_unique<Buffer>(
            device,
            indexSize,
            mIndexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        stagingBuffer.CopyBuffer(stagingBuffer.GetBuffer(), mIndexBuffer->GetBuffer(), bufferSize);
    }

    void Mesh::Bind(VkCommandBuffer commandBuffer)
    {
        if (!mHasIndexBuffer)
        {
            DOG_CRITICAL("Binding but no index buffer! Should not be happening");
            return;
        }

        VkBuffer buffers[] = { mVertexBuffer->GetBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }

    void Mesh::Draw(VkCommandBuffer commandBuffer, uint32_t baseIndex)
    {
        if (!mHasIndexBuffer)
        {
            DOG_CRITICAL("Drawing with no index buffer! Should not be happening");
            return;
        }

        vkCmdDrawIndexed(commandBuffer, mIndexCount, 1, 0, 0, baseIndex);
    }

    std::vector<VkVertexInputBindingDescription> Vertex::GetBindingDescriptions()
    {
        //Create a 1 long vector of binding descriptions
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);

        //Set bind description data
        bindingDescriptions[0].binding = 0;                             //Set binding location
        bindingDescriptions[0].stride = sizeof(Vertex);                 //Size stride to the size of a vertex
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; //Specifys whether vertex attribute addressing is a function of the vertex index or of the instance index (using vertex)

        //Return description
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> Vertex::GetAttributeDescriptions()
    {
        //Create a vector of attribute descriptions
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        //Set attribute description data for the position
        //Sets (location of this attribute (matches what is defined at top of shader, binding location, 
        //      datatype to three 32bit signed floats (vec3), the offset into the Vector struct for the position varible)
        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });

        //Set attribute description data for the color
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });

        //Set attribute description data for normals
        attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });

        //Set attribute description data for texture coords
        attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });

        //Set attribute description data for bone IDs
        attributeDescriptions.push_back({ 4, 0, VK_FORMAT_R32G32B32A32_SINT, offsetof(Vertex, boneIDs) });

        //Set attribute description data for bone weights
        attributeDescriptions.push_back({ 5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, weights) });

        //Return description
        return attributeDescriptions;
    }

    void Vertex::SetBoneData(int boneID, float weight)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) 
        {
            if (weights[i] == 0.0f)
            {
                boneIDs[i] = boneID;
                weights[i] = weight;
                return;
            }
            if (i == MAX_BONE_INFLUENCE - 1) 
            {
                __debugbreak();
            }
        }
    }

}
