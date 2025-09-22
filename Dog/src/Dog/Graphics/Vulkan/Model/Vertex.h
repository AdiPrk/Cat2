#pragma once

namespace Dog
{
    struct Vertex
    {
        glm::vec3 position{ 0.f }; //Position of this vertex
        glm::vec3 color{ 1.f };    //Color of this vertex
        glm::vec3 normal{ 0.f };   //Normal of this vertex
        glm::vec2 uv{ 0.f };       //Texture coords of this vertex

        static constexpr int MAX_BONE_INFLUENCE = 4;

        std::array<int, MAX_BONE_INFLUENCE> boneIDs = { -1, -1, -1, -1 };
        std::array<float, MAX_BONE_INFLUENCE> weights = { 0.0f, 0.0f, 0.0f, 0.0f };

        static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

        void SetBoneData(int boneID, float weight);
    };
}
