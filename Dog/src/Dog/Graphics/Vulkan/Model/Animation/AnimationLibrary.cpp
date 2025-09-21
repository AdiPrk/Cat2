#include <PCH/pch.h>
#include "AnimationLibrary.h"
#include "Animator.h"

namespace Dog
{
    const uint32_t AnimationLibrary::INVALID_ANIMATION_INDEX = 10001;

    AnimationLibrary::AnimationLibrary()
    {
    }

    AnimationLibrary::~AnimationLibrary()
    {
    }

    uint32_t AnimationLibrary::AddAnimation(const std::string& animPath, Model* model)
    {
        static Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(animPath, aiProcessPreset_TargetRealtime_MaxQuality);

        if (!scene || !scene->mAnimations[0] || !scene->mRootNode)
        {
            DOG_CRITICAL("Failed to load animation at path: {0}", animPath);
            return INVALID_ANIMATION_INDEX;
        }

        uint32_t animationID = static_cast<uint32_t>(mAnimation.size());
        
        mAnimation.emplace_back(std::make_unique<Animation>(scene, model));
        mAnimators.emplace_back(std::make_unique<Animator>(mAnimation.back().get()));

        std::string mAnimName = std::filesystem::path(animPath).stem().string();
        mAnimationMap[mAnimName] = animationID;

        return animationID;
    }

    Animation* AnimationLibrary::GetAnimation(uint32_t index)
    {
        if (index >= mAnimation.size())
        {
            DOG_WARN("Animation ID {0} is out of range!", index);
            return nullptr;
        }

        return mAnimation[index].get();
    }

    Animator* AnimationLibrary::GetAnimator(uint32_t index)
    {
        if (index >= mAnimators.size())
        {
            DOG_WARN("Animator ID {0} is out of range!", index);
            return nullptr;
        }
        return mAnimators[index].get();
    }

    const std::vector<glm::mat4>& AnimationLibrary::GetAnimationMatrices()
    {
        mAnimationMatrices.clear();

        for (const auto& animator : mAnimators)
        {
            const auto& finalMatrices = animator->GetFinalBoneMatrices();
            mAnimationMatrices.insert(mAnimationMatrices.end(), finalMatrices.begin(), finalMatrices.end());
        }

        return mAnimationMatrices;
    }

    void AnimationLibrary::UpdateAnimations(float dt)
    {
        for (const auto& animator : mAnimators)
        {
            if (!animator->IsPlaying()) continue;
            animator->UpdateAnimation(dt);
        }
    }


    void AnimationLibrary::UpdateAnimation(uint32_t index, float dt)
    {
        if (index >= mAnimators.size())
        {
            DOG_CRITICAL("Animator ID {0} is out of range!", index);
            return;
        }

        mAnimators[index]->UpdateAnimation(dt);
    }
}
