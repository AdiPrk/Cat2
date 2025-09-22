/*****************************************************************//**
 * \file   Animation.hpp
 * \author Adi (aditya.prakash@digipen.edu)
 * \date   November 6 2024
 * \Copyright @ 2024 Digipen (USA) Corporation * 
 
 * \brief  Sets up the アニメーション data
 *  *********************************************************************/

#pragma once

#include "../Model.h"
#include "Bone.h"

namespace Dog
{
    // This struct is used when creating the animation data
    struct AnimationNode
    {
        VQS transformation;
        int id;
        std::vector<int> childIndices;
    };

    class Animation
    {
    public:
        Animation();
        Animation(const aiScene* scene, Model* model);
        ~Animation() {}

        Bone* FindBone(int id);

        float GetTicksPerSecond() const { return mTicksPerSecond; }
        float GetDuration() const { return mDuration; }
        const auto& GetBoneIDMap() { return mBoneInfoMap; }
        const auto& GetBoneMap() { return mBoneMap; }

        const std::vector<AnimationNode>& GetNodes() const { return mNodes; }
        const AnimationNode& GetNode(int index) const { return mNodes[index]; }
        int GetRootNodeIndex() const { return mRootNodeIndex; }

        // AI_SBBC_DEFAULT_MAX_BONES is defined in pch.h so that assimp builds correctly
        static const uint32_t MAX_BONES = AI_SBBC_DEFAULT_MAX_BONES;

    private:
        void ReadMissingBones(const aiAnimation* animation, Model& model);
        void ReadHeirarchyData(int parentIndex, const aiNode* src);

        float mDuration;
        float mTicksPerSecond;

        std::unordered_map<int, Bone> mBoneMap;
        std::unordered_map<int, BoneInfo> mBoneInfoMap;
        std::unordered_map<std::string, int> mNameToIDMap;

        std::vector<AnimationNode> mNodes;
        int nodeCounter;
        int mRootNodeIndex;
    };

}
