#include <PCH/pch.h>
#include "Animator.h"
#include "Animation.h"

namespace Dog
{
  Animator::Animator(Animation* animation)
  {
    mCurrentTime = 0.0;
    mCurrentAnimation = animation;

    mFinalBoneMatrices.resize(Animation::MAX_BONES);
    std::fill(mFinalBoneMatrices.begin(), mFinalBoneMatrices.end(), glm::mat4(1.0f));
  }

  void Animator::UpdateAnimation(float dt)
  {
    if (mCurrentAnimation)
    {
      mCurrentTime += mCurrentAnimation->GetTicksPerSecond() * dt;
      mCurrentTime = fmod(mCurrentTime, mCurrentAnimation->GetDuration());
      CalculateBoneTransform(mCurrentAnimation->GetRootNodeIndex(), glm::mat4(1.0f));
    }
  }

  void Animator::PlayAnimation(Animation* pAnimation)
  {
    mCurrentAnimation = pAnimation;
    mCurrentTime = 0.0f;
  }

  void Animator::CalculateBoneTransform(int nodeIndex, const glm::mat4& parentTransform)
  {
    const AnimationNode& node = mCurrentAnimation->GetNode(nodeIndex);
    int nodeId = node.id;
    const glm::mat4* nodeTransform = nullptr;

    if (Bone* Bone = mCurrentAnimation->FindBone(nodeId))
    {
      Bone->Update(mCurrentTime);
      nodeTransform = &Bone->GetLocalTransform();
    }
    else
    {
      nodeTransform = &node.transformation;
    }

    glm::mat4 globalTransformation = parentTransform * *nodeTransform;

    const auto& boneInfoMap = mCurrentAnimation->GetBoneIDMap();

    auto boneIt = boneInfoMap.find(nodeId);
    if (boneIt != boneInfoMap.end())
    {
      const BoneInfo& info = boneIt->second;
      mFinalBoneMatrices[info.id] = globalTransformation * info.offset;
    }

    for (int childIndex : node.childIndices)
    {
      CalculateBoneTransform(childIndex, globalTransformation);
    }
  }

} // namespace Rendering
