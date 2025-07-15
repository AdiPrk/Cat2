#pragma once

#include "../Buffers/Buffer.h"
#include "../Core/Device.h"
#include "Core/Scene/Entity/Components.h"

namespace Dog {

    struct MaterialComponent;

    struct Vertex {
        glm::vec3 position{};
        glm::vec3 color{};
        glm::vec3 normal{};
        glm::vec2 uv{};

        std::array<int, MAX_BONE_INFLUENCE> boneIDs = { -1, -1, -1, -1 };
        std::array<float, MAX_BONE_INFLUENCE> weights = { 0.0f, 0.0f, 0.0f, 0.0f };

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        void SetBoneData(int boneID, float weight);
    };

    class Mesh {
    public:
        Mesh(bool assignID = true);

        void createVertexBuffers(Device& device);
        void createIndexBuffers(Device& device);
        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

        std::unique_ptr<Buffer> vertexBuffer;
        uint32_t vertexCount = 0;

        bool hasIndexBuffer = false;
        std::unique_ptr<Buffer> indexBuffer;
        uint32_t indexCount = 0;

        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};
        
        MaterialComponent materialComponent{};

        // unique index for this mesh
        uint32_t mMeshID = 0;

        static int GetTotalMeshCount() { return uniqueMeshIndex; }

    private:
        static int uniqueMeshIndex;
    };

} // namespace Dog