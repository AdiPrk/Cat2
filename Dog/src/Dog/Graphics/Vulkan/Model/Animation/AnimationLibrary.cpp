#include <PCH/pch.h>
#include "AnimationLibrary.h"
#include "Animator.h"

namespace Dog
{
    const uint32_t AnimationLibrary::INVALID_ANIMATION_INDEX = 9999;

    AnimationLibrary::AnimationLibrary()
    {
    }

    AnimationLibrary::~AnimationLibrary()
    {
    }

    uint32_t AnimationLibrary::AddAnimation(const std::string& animPath, Model* model)
    {
        uint32_t animationID = static_cast<uint32_t>(mAnimation.size());
        
        mAnimation.emplace_back(std::make_unique<Animation>(animPath, model->mScene, model));
        mAnimators.emplace_back(std::make_unique<Animator>(mAnimation.back().get()));

        mAnimationMap[animPath] = animationID;

        return animationID;
    }

    Animation* AnimationLibrary::GetAnimation(uint32_t index)
    {
        if (index >= mAnimation.size())
        {
            DOG_CRITICAL("Animation ID {0} is out of range!", index);
            return nullptr;
        }
        return mAnimation[index].get();
    }

    Animator* AnimationLibrary::GetAnimator(uint32_t index)
    {
        if (index >= mAnimators.size())
        {
            DOG_CRITICAL("Animator ID {0} is out of range!", index);
            return nullptr;
        }
        return mAnimators[index].get();
    }
}
