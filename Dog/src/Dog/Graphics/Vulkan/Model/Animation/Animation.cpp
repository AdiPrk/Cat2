#include <PCH/pch.h>
#include "Animation.h"

namespace Dog
{
    // Dump all nodes (hierarchical)
    void DumpNodeTree(const aiNode* node, int depth = 0)
    {
        std::string indent(depth * 2, ' ');
        DOG_INFO("{}Node: '{}'", indent, node->mName.C_Str());
        for (unsigned i = 0; i < node->mNumChildren; ++i)
            DumpNodeTree(node->mChildren[i], depth + 1);
    }

    void DumpModelInfo(const Model& model)
    {
        const aiScene* s = model.mScene;
        DOG_INFO("== Model Scene Node Tree ==");
        DumpNodeTree(s->mRootNode);

        auto& boneInfo = model.GetBoneInfoMap();
        DOG_INFO("== Model BoneInfoMap (name -> id) ==");
        for (auto& p : boneInfo)
            DOG_INFO(" bone '{}' -> id {} (offset matrix present?)", p.first, p.second.id);
    }

    // Dump animation channels
    void DumpAnimationChannels(const aiScene* animScene)
    {
        if (!animScene || animScene->mNumAnimations == 0) { DOG_INFO("No animations"); return; }
        aiAnimation* a = animScene->mAnimations[0];
        DOG_INFO("Animation duration {} ticks, ticksPerSecond {}", a->mDuration, a->mTicksPerSecond);
        DOG_INFO("Channels: {}", a->mNumChannels);
        for (unsigned i = 0; i < a->mNumChannels; ++i)
        {
            aiNodeAnim* ch = a->mChannels[i];
            DOG_INFO(" channel[{}] -> nodeName '{}'", i, ch->mNodeName.C_Str());
            DOG_INFO("   numPosKeys {}, numRotKeys {}, numScaleKeys {}",
                ch->mNumPositionKeys, ch->mNumRotationKeys, ch->mNumScalingKeys);
        }
    }

    Animation::Animation()
        : mDuration(0.0f)
        , mTicksPerSecond(30.f)
        , nodeCounter(0)
        , mRootNodeIndex(0)
    {
    }

    Animation::Animation(const aiScene* scene, Model* model)
        : mDuration(0.0f)
        , mTicksPerSecond(30.f)
        , nodeCounter(0)
        , mRootNodeIndex(0)
    {
        // Dump stuff!
        //DumpModelInfo(*model);
        
        DumpAnimationChannels(scene);

        const aiScene* modelScene = model->mScene; // <- model's scene, accessible if we need
        // scene <- the animation file's scene

        // Load the first animation
        aiAnimation* animation = scene->mAnimations[0];

        mDuration = static_cast<float>(animation->mDuration);
        if (animation->mTicksPerSecond != 0)
        {
            mTicksPerSecond = static_cast<float>(animation->mTicksPerSecond);
        }

        // Scale model to unit size (only works properly on .glb for now)
        {
            mNodes.clear();

            AnimationNode node;
            VQS rootVQS(aiMatToGlm(modelScene->mRootNode->mTransformation));

            VQS scaleVQS;
            scaleVQS.scale = glm::vec3(model->GetModelScale());

            VQS centerVQS;
            centerVQS.translation = -model->GetModelCenter();

            VQS finalVQS = rootVQS * scaleVQS * centerVQS;

            node.transformation = finalVQS;
            mNodes.push_back(node);
        }


        ReadHeirarchyData(-1, modelScene->mRootNode);
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
            node.transformation = aiMatToGlm(src->mTransformation);
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
        std::unordered_map<std::string, BoneInfo>& modelBoneInfo = model.GetBoneInfoMap();
        int& boneCount = model.GetBoneCount();

        for (unsigned i = 0; i < animation->mNumChannels; ++i)
        {
            aiNodeAnim* channel = animation->mChannels[i];
            const std::string boneName = channel->mNodeName.data;

            // find node id in the *model* node map (built from model's scene!)
            auto nameIt = mNameToIDMap.find(boneName);
            if (nameIt == mNameToIDMap.end())
            {
                DOG_WARN("Animation channel '{0}' not found in model node map, skipping", boneName);
                continue;
            }
            int nodeId = nameIt->second;

            // find the BoneInfo that was created when we parsed the model's meshes
            auto boneIt = modelBoneInfo.find(boneName);
            if (boneIt == modelBoneInfo.end())
            {
                DOG_WARN("Animation references bone '{0}' not present in model bone map, skipping", boneName);
                continue;
            }
            int boneIndex = boneIt->second.id;

            // finally add the bone to this Animation's bone map (keyed by nodeId,
            // storing model's bone index and the aiNodeAnim* so it can evaluate)
            mBoneMap.try_emplace(nodeId, boneIndex, channel);
        }

        // recreate nodeId->BoneInfo map (if you need it) using the model's bone info
        mBoneInfoMap.clear();
        for (const auto& pair : modelBoneInfo)
        {
            const std::string& name = pair.first;
            int nodeId = mNameToIDMap.count(name) ? mNameToIDMap[name] : -1;
            if (nodeId >= 0)
                mBoneInfoMap[nodeId] = pair.second;
        }
    }


} // namespace Rendering
