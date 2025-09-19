#include <PCH/pch.h>
#include "Bone.h"
#include <concepts>

/*reads keyframes from aiNodeAnim*/
namespace Dog
{
    // default ctor required for some stl containers
    Bone::Bone()
        : mID(-1)
        , mLocalTransform(1.0f)
        , mPositionMatrix(1.0f)
        , mRotationMatrix(1.0f)
        , mScalingMatrix(1.0f)
    {
    }

    Bone::Bone(int ID)
        : mID(ID)
        , mLocalTransform(1.0f)
        , mPositionMatrix(1.0f)
        , mRotationMatrix(1.0f)
        , mScalingMatrix(1.0f)
    {
    }

    Bone::Bone(int ID, const aiNodeAnim* channel)
        : mID(ID)
        , mLocalTransform(1.0f)
        , mPositionMatrix(1.0f)
        , mRotationMatrix(1.0f)
        , mScalingMatrix(1.0f)
    {
        for (int positionIndex = 0; positionIndex < channel->mNumPositionKeys; ++positionIndex)
        {
            aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
            float time = static_cast<float>(channel->mPositionKeys[positionIndex].mTime);

            mPositions.emplace_back(aiVecToGlm(aiPosition), time);
        }

        for (int rotationIndex = 0; rotationIndex < channel->mNumRotationKeys; ++rotationIndex)
        {
            aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
            float time = static_cast<float>(channel->mRotationKeys[rotationIndex].mTime);

            mRotations.emplace_back(aiQuatToGlm(aiOrientation), time);
        }

        for (int keyIndex = 0; keyIndex < channel->mNumScalingKeys; ++keyIndex)
        {
            aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
            float time = static_cast<float>(channel->mScalingKeys[keyIndex].mTime);

            mScales.emplace_back(aiVecToGlm(scale), time);
        }
    }

    void Bone::Update(float animationTime)
    {
        InterpolatePosition(animationTime);
        InterpolateRotation(animationTime);
        InterpolateScaling(animationTime);
    
        mLocalTransform = mPositionMatrix * mRotationMatrix * mScalingMatrix;
    }

    template<typename T>
    concept Keyframe = requires(const T& k) {
        { k.time } -> std::convertible_to<float>; 
    };

    template<Keyframe KeyframeType>
    constexpr size_t FindKeyframeIndex(float animationTime, std::span<const KeyframeType> keyframes)
    {
        if (keyframes.size() <= 1) 
        {
            return 0;
        }

        const auto it = std::ranges::lower_bound(keyframes, animationTime, {}, &KeyframeType::time);
        const auto index = static_cast<size_t>(it - keyframes.begin());

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
            mPositionMatrix = glm::translate(glm::mat4(1.0f), mPositions[0].position);
            return;
        }

        int p0Index = FindKeyframeIndex<KeyPosition>(animationTime, mPositions);
        int p1Index = p0Index + 1;

        float scaleFactor = GetScaleFactor(mPositions[p0Index].time, mPositions[p1Index].time, animationTime);
        glm::vec3 finalPosition = glm::mix(mPositions[p0Index].position, mPositions[p1Index].position, scaleFactor);

        mPositionMatrix = glm::translate(glm::mat4(1.0f), finalPosition);
    }

    void Bone::InterpolateRotation(float animationTime)
    {
        if (mRotations.size() == 1)
        {
            mRotationMatrix = glm::toMat4(glm::normalize(mRotations[0].orientation));
            return;
        }

        int p0Index = FindKeyframeIndex<KeyRotation>(animationTime, mRotations);
        int p1Index = p0Index + 1;

        float scaleFactor = GetScaleFactor(mRotations[p0Index].time, mRotations[p1Index].time, animationTime);
        glm::quat finalRotation = glm::slerp(mRotations[p0Index].orientation, mRotations[p1Index].orientation, scaleFactor);

        mRotationMatrix = glm::toMat4(glm::normalize(finalRotation));
    }

    void Bone::InterpolateScaling(float animationTime)
    {
        if (mScales.size() == 1)
        {
            mScalingMatrix = glm::scale(glm::mat4(1.0f), mScales[0].scale);
            return;
        }

        int p0Index = FindKeyframeIndex<KeyScale>(animationTime, mScales);
        int p1Index = p0Index + 1;

        float scaleFactor = GetScaleFactor(mScales[p0Index].time, mScales[p1Index].time, animationTime);
        glm::vec3 finalScale = glm::mix(mScales[p0Index].scale, mScales[p1Index].scale, scaleFactor);

        mScalingMatrix = glm::scale(glm::mat4(1.0f), finalScale);
    }
}