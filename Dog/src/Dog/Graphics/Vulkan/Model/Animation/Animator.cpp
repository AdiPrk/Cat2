#include <PCH/pch.h>
#include "Animator.h"
#include "Animation.h"

namespace Dog
{
  Animator::Animator(Animation* animation)
  {
    mCurrentTime = 0.0;
    mCurrentAnimation = animation;

    // resize the final bone matrices based on biggest bone ID in the animation
    int maxBoneID = 0;
    for (const auto& pair : mCurrentAnimation->GetBoneIDMap())
    {
        if (pair.second.id > maxBoneID)
        {
            maxBoneID = pair.second.id;
        }
    }

    mFinalBoneVQS.resize(maxBoneID + 1);
  }

  void Animator::UpdateAnimation(float dt)
  {
    if (mCurrentAnimation)
    {
      mCurrentTime += mCurrentAnimation->GetTicksPerSecond() * dt;
      mCurrentTime = fmod(mCurrentTime, mCurrentAnimation->GetDuration());
      VQS identity;
      CalculateBoneTransform(mCurrentAnimation->GetRootNodeIndex(), identity);
    }
  }

  void Animator::UpdateAnimationInstant(float time)
  {
      if (mCurrentAnimation)
      {
          mCurrentTime = time;
          VQS identity;
          CalculateBoneTransform(mCurrentAnimation->GetRootNodeIndex(), identity);
      }
  }

  void Animator::PlayAnimation(Animation* pAnimation)
  {
    mCurrentAnimation = pAnimation;
    mCurrentTime = 0.0f;
  }

  void Animator::CalculateBoneTransform(int nodeIndex, const VQS& parentTransform)
  {
    const AnimationNode& node = mCurrentAnimation->GetNode(nodeIndex);
    int nodeId = node.id;
    VQS nodeTransform;

    if (Bone* Bone = mCurrentAnimation->FindBone(nodeId))
    {
      Bone->Update(mCurrentTime);
      nodeTransform = Bone->GetLocalTransform();
    }
    else
    {
      nodeTransform = node.transformation;
    }

    VQS globalTransformation = compose(parentTransform, nodeTransform);

    const auto& boneInfoMap = mCurrentAnimation->GetBoneIDMap();
    auto boneIt = boneInfoMap.find(nodeId);
    if (boneIt != boneInfoMap.end())
    {
        const BoneInfo& info = boneIt->second;
        mFinalBoneVQS[info.id] = compose(globalTransformation, info.vqsOffset);
    }

    for (int childIndex : node.childIndices)
    {
      CalculateBoneTransform(childIndex, globalTransformation);
    }
  }

} // namespace Rendering
