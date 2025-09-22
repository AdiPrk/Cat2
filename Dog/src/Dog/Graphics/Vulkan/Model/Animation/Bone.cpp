#include <PCH/pch.h>
#include "Bone.h"
#include <concepts>

/*reads keyframes from aiNodeAnim*/
namespace Dog
{
    // default ctor required for some stl containers
    Bone::Bone()
        : mID(-1)
        , mLocalTransform()
    {
    }

    Bone::Bone(int ID)
        : mID(ID)
        , mLocalTransform()
    {
    }

    Bone::Bone(int ID, const aiNodeAnim* channel)
        : mID(ID)
        , mLocalTransform()
    {
        for (unsigned ind = 0; ind < channel->mNumPositionKeys; ++ind)
        {
            aiVector3D aiPosition = channel->mPositionKeys[ind].mValue;
            float time = static_cast<float>(channel->mPositionKeys[ind].mTime);

            mPositions.emplace_back(aiVecToGlm(aiPosition), time);
        }

        for (unsigned ind = 0; ind < channel->mNumRotationKeys; ++ind)
        {
            aiQuaternion aiOrientation = channel->mRotationKeys[ind].mValue;
            float time = static_cast<float>(channel->mRotationKeys[ind].mTime);

            mRotations.emplace_back(aiQuatToGlm(aiOrientation), time);
        }

        for (unsigned ind = 0; ind < channel->mNumScalingKeys; ++ind)
        {
            aiVector3D scale = channel->mScalingKeys[ind].mValue;
            float time = static_cast<float>(channel->mScalingKeys[ind].mTime);

            mScales.emplace_back(aiVecToGlm(scale), time);
        }
    }

    void Bone::Update(float animationTime)
    {
        InterpolatePosition(animationTime);
        InterpolateRotation(animationTime);
        InterpolateScaling(animationTime);
    }

    template<typename T>
    concept Keyframe = requires(const T& k) {
        { k.time } -> std::convertible_to<float>; 
    };

    template<Keyframe KeyframeType>
    constexpr int FindKeyframeIndex(float animationTime, std::span<const KeyframeType> keyframes)
    {
        if (keyframes.size() <= 1) 
        {
            return 0;
        }

        const auto it = std::ranges::lower_bound(keyframes, animationTime, {}, &KeyframeType::time);
        const int index = static_cast<int>(it - keyframes.begin());

        return (index > 0) ? (index - 1) : 0;
    }

    float Bone::GetScaleFactor(float lastTime, float nextTime, float animationTime) const
    {
        float midWayLength = animationTime - lastTime;
        float framesDiff = nextTime - lastTime;
        if (framesDiff == 0.0f)
        {
            DOG_WARN("Division by 0 in Bone::GetScaleFactor");
            return 0.0f;
        }
        return midWayLength / framesDiff;
    }

    void Bone::InterpolatePosition(float animationTime)
    {
        if (mPositions.size() == 1)
        {
            mLocalTransform.translation = mPositions[0].position;
            return;
        }

        int p0Index = FindKeyframeIndex<KeyPosition>(animationTime, mPositions);
        int p1Index = p0Index + 1;
        if (p1Index >= mPositions.size())
            p1Index = 0;

        float scaleFactor = GetScaleFactor(mPositions[p0Index].time, mPositions[p1Index].time, animationTime);
        mLocalTransform.translation = glm::mix(mPositions[p0Index].position, mPositions[p1Index].position, scaleFactor);
    }

    void Bone::InterpolateRotation(float animationTime)
    {
        if (mRotations.size() == 1)
        {
            mLocalTransform.rotation = glm::normalize(mRotations[0].orientation);
            return;
        }

        int p0Index = FindKeyframeIndex<KeyRotation>(animationTime, mRotations);
        int p1Index = p0Index + 1;
        if (p1Index >= mRotations.size())
            p1Index = 0;

        float scaleFactor = GetScaleFactor(mRotations[p0Index].time, mRotations[p1Index].time, animationTime);

        mLocalTransform.rotation = glm::slerp(mRotations[p0Index].orientation, mRotations[p1Index].orientation, scaleFactor);
        mLocalTransform.rotation = glm::normalize(mLocalTransform.rotation); // Normalize after slerp
    }

    void Bone::InterpolateScaling(float animationTime)
    {
        if (mScales.size() == 1)
        {
            mLocalTransform.scale = mScales[0].scale;
            return;
        }

        int p0Index = FindKeyframeIndex<KeyScale>(animationTime, mScales);
        int p1Index = p0Index + 1;
        if (p1Index >= mScales.size())
            p1Index = 0;

        float scaleFactor = GetScaleFactor(mScales[p0Index].time, mScales[p1Index].time, animationTime);
        mLocalTransform.scale = glm::mix(mScales[p0Index].scale, mScales[p1Index].scale, scaleFactor);
    }
}