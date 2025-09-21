#include <PCH/pch.h>
#include "Animation.h"

namespace Dog
{
    Animation::Animation()
        : mDuration(0.0f)
        , mTicksPerSecond(30)
        , nodeCounter(0)
        , mRootNodeIndex(0)
    {
    }

    Animation::Animation(const aiScene* scene, Model* model)
        : mDuration(0.0f)
        , mTicksPerSecond(30)
        , nodeCounter(0)
        , mRootNodeIndex(0)
    {
        // Load the first animation
        aiAnimation* animation = scene->mAnimations[0];

        mDuration = static_cast<float>(animation->mDuration);
        if (animation->mTicksPerSecond != 0)
        {
            mTicksPerSecond = static_cast<int>(animation->mTicksPerSecond);
        }

        // Scale model to unit size (only works properly on .glb for now)
        {
            mNodes.clear();

            AnimationNode node;
            glm::mat4 rootTransformMat = aiMatToGlm(scene->mRootNode->mTransformation);

            // 1. Decompose the root node's transform into an initial VQS
            VQS rootVQS;
            {
                glm::vec3 s, t, sk;
                glm::quat r;
                glm::vec4 p;
                glm::decompose(rootTransformMat, s, r, t, sk, p);
                rootVQS.scale = s;
                rootVQS.rotation = r;
                rootVQS.translation = t;
            }

            // 2. Create VQS structs for the scale and center adjustments
            VQS scaleVQS;
            scaleVQS.scale = glm::vec3(model->GetModelScale());

            VQS centerVQS;
            centerVQS.translation = -model->GetModelCenter();

            // 3. Compose the transforms together using your VQS compose function
            // The order is the same: root * scale * center
            VQS finalVQS = compose(compose(rootVQS, scaleVQS), centerVQS);

            // 4. Store the final result
            node.transformation = finalVQS;
            mNodes.push_back(node);
        }


        ReadHeirarchyData(-1, scene->mRootNode);
        ReadMissingBones(animation, *model);       

        // clear data that we don't need anymore
        mNameToIDMap.clear();
    }

    Bone* Animation::FindBone(int id)
    {
        auto iter = mBoneMap.find(id);
        if (iter == mBoneMap.end()) return nullptr;
        else return &iter->second;
    }

    void Animation::ReadHeirarchyData(int parentIndex, const aiNode* src)
    {
        std::string nodeName = src->mName.data;

        int nodeId;
        auto it = mNameToIDMap.find(nodeName);
        if (it == mNameToIDMap.end())
        {
            nodeId = nodeCounter++;
            mNameToIDMap[nodeName] = nodeId;
        }
        else
        {
            nodeId = it->second;
        }

        int currentIndex = static_cast<int>(mNodes.size());

        if (parentIndex != -1)
        {
            AnimationNode node;
            node.id = nodeId;

            glm::mat4 transform = aiMatToGlm(src->mTransformation);

            VQS vqs;
            {
                glm::vec3 s, t, sk;
                glm::quat r;
                glm::vec4 p;
                glm::decompose(transform, s, r, t, sk, p);
                vqs.scale = s;
                vqs.rotation = r;
                vqs.translation = t;
            }

            node.transformation = vqs;
            mNodes.push_back(node);

            mNodes[parentIndex].childIndices.push_back(currentIndex);
        }
        else
        {
            currentIndex = 0;
            mNodes[currentIndex].id = nodeId;
        }

        // Recursively read children
        for (unsigned int i = 0; i < src->mNumChildren; i++)
        {
            ReadHeirarchyData(currentIndex, src->mChildren[i]);
        }
    }

    // 
    void Animation::ReadMissingBones(const aiAnimation* animation, Model& model)
    {
        std::unordered_map<std::string, BoneInfo>& boneInfoMap = model.GetBoneInfoMap();
        int& boneCount = model.GetBoneCount();

        for (unsigned i = 0; i < animation->mNumChannels; i++)
        {
            aiNodeAnim* channel = animation->mChannels[i];
            const std::string& boneName = channel->mNodeName.data;

            int nodeId = mNameToIDMap[boneName];

            if (boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                boneInfoMap[boneName].id = boneCount++;
            }

            mBoneMap.try_emplace(nodeId, boneInfoMap[boneName].id, channel);
        }

        // Map from node IDs to BoneInfo
        mBoneInfoMap.clear();
        for (const auto& pair : boneInfoMap)
        {
            const std::string& boneName = pair.first;
            int nodeId = mNameToIDMap[boneName];
            mBoneInfoMap[nodeId] = pair.second;
        }
    }

} // namespace Rendering
