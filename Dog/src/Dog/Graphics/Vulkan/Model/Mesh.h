#pragma once

#include "Vertex.h"

namespace Dog
{
    // Forward reference
    class Device;
    class Buffer;
    struct RenderingResource;

    class Mesh {
    public:
        Mesh(bool assignID = true);

        void CreateVertexBuffers(Device& device);
        void CreateIndexBuffers(Device& device);

        void Bind(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer, uint32_t baseIndex = 0);

        std::unique_ptr<Buffer> mVertexBuffer;
        uint32_t mVertexCount = 0;

        bool mHasIndexBuffer = false;
        std::unique_ptr<Buffer> mIndexBuffer;
        uint32_t mIndexCount = 0;

        std::vector<Vertex> mVertices{};
        std::vector<uint32_t> mIndices{};

        // Tex data if from memory
        std::unique_ptr<unsigned char[]> mTextureData = nullptr;
        uint32_t mTextureSize = 0;
        bool mTextureLoaded = false;
        uint32_t mWidth, mHeight, mChannels;

        // unique index for this mesh
        uint32_t mMeshID = 0;

        // Path to textures
        std::string diffuseTexturePath = "";
        uint32_t diffuseTextureIndex = -1;
        
    private:
        static int GetTotalMeshCount() { return uniqueMeshIndex; }
        static int uniqueMeshIndex;
    };
}
