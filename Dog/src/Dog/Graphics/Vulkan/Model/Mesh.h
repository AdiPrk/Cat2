#pragma once

//#include "../Buffer.hpp"

namespace Client
{
    // Forward References
    struct RenderingResource;
}

namespace Dog
{
    // Forward reference
    class Device;
    class Buffer;

    struct Vertex
    {
        glm::vec3 position{}; //Position of this vertex
        glm::vec3 color{};    //Color of this vertex
        glm::vec3 normal{};   //Normal of this vertex
        glm::vec2 uv{};       //Texture coords of this vertex

        static constexpr int MAX_BONE_INFLUENCE = 4;

        std::array<int, MAX_BONE_INFLUENCE> boneIDs = { -1, -1, -1, -1 };
        std::array<float, MAX_BONE_INFLUENCE> weights = { 0.0f, 0.0f, 0.0f, 0.0f };

        static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

        void SetBoneData(int boneID, float weight);
    };

    class Mesh {
    public:
        Mesh(bool assignID = true);

        void CreateVertexBuffers(Device& device);
        void CreateIndexBuffers(Device& device);

        void Bind(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer);

        std::unique_ptr<Buffer> mVertexBuffer;
        uint32_t mVertexCount = 0;

        bool mHasIndexBuffer = false;
        std::unique_ptr<Buffer> mIndexBuffer;
        uint32_t mIndexCount = 0;

        std::vector<Vertex> mVertices{};
        std::vector<uint32_t> mIndices{};

        uint32_t bakedTextureIndex = -1;

        // unique index for this mesh
        uint32_t mMeshID = 0;


    private:
        static int GetTotalMeshCount() { return uniqueMeshIndex; }
        static int uniqueMeshIndex;
    };
}
