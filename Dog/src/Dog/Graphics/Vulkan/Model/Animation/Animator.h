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

    void PlayAnimation(Animation* pAnimation);

    void CalculateBoneTransform(int nodeIndex, const glm::mat4& parentTransform);

    const auto& GetFinalBoneMatrices() const { return mFinalBoneMatrices; }

  private:
    std::array<glm::mat4, Animation::MAX_BONES> mFinalBoneMatrices;
    Animation* mCurrentAnimation;
    float mCurrentTime;
  };

} // namespace Rendering
