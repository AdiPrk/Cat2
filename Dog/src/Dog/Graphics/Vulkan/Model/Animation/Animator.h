/*****************************************************************//**
 * \file   Animator.hpp
 * \author Adi (aditya.prakash@digipen.edu)
 * \date   November 6 2024
 * \Copyright @ 2024 Digipen (USA) Corporation * 
 
 * \brief  Animates an animation!!!
 *  *********************************************************************/

#pragma once

#include "Animation.h"

namespace Dog
{
  class Animator
  {
  public:
    Animator(Animation* animation);

    void UpdateAnimation(float dt);
    void UpdateAnimationInstant(float time);
    void PlayAnimation(Animation* pAnimation);
    void CalculateBoneTransform(int nodeIndex, const VQS& parentTransform);

    const std::vector<VQS>& GetFinalBoneVQS() const { return mFinalBoneVQS; }
    bool IsPlaying() const { return mIsPlaying; }

  private:
    std::vector<VQS> mFinalBoneVQS;

    Animation* mCurrentAnimation;
    float mCurrentTime;

    bool mIsPlaying = true;
  };

} // namespace Rendering
