/*****************************************************************//**
 * \file   Bone.hpp
 * \author Adi (aditya.prakash@digipen.edu)
 * \date   November 6 2024
 * \Copyright @ 2024 Digipen (USA) Corporation *

 * \brief  Holds bone data for skeletal animation
 *  *********************************************************************/

#pragma once

namespace Dog
{
    struct BoneInfo
    {
        int id; // index into finalBoneMatrices
        glm::mat4 offset; // offset from model to bone space

        BoneInfo() : id(0), offset(1.0f) {}
        BoneInfo(int ID, const glm::mat4& offsetMatrix) : id(ID), offset(offsetMatrix) {}
    };

    struct KeyPosition
    {
        glm::vec3 position;
        float time;

        KeyPosition() : position(0.0f), time(0.0f) {}
        KeyPosition(const glm::vec3& pos, float timeStamp) : position(pos), time(timeStamp) {}
    };

    struct KeyRotation
    {
        glm::quat orientation;
        float time;

        KeyRotation() : orientation(1.0f, 0.0f, 0.0f, 0.0f), time(0.0f) {}
        KeyRotation(const glm::quat& ori, float timeStamp) : orientation(ori), time(timeStamp) {}
    };

    struct KeyScale
    {
        glm::vec3 scale;
        float time;

        KeyScale() : scale(1.0f), time(0.0f) {}
        KeyScale(const glm::vec3& scale, float timeStamp) : scale(scale), time(timeStamp) {}
    };

    class Bone
    {
    public:
        // Reads bone keyframe data from the aiNodeAnim
        Bone();
        Bone(int ID);
        Bone(int ID, const aiNodeAnim* channel);

        // Interpolates transformation matrix based on current animation time
        void Update(float animationTime);

        inline const glm::mat4& GetLocalTransform() const noexcept { return mLocalTransform; }

        int GetBoneID() const { return mID; }


        /* Gets the current index on mKeyPositions to interpolate to based on
        the current animation time*/
        int GetPositionIndex(float animationTime);

        /* Gets the current index on mKeyRotations to interpolate to based on the
        current animation time*/
        int GetRotationIndex(float animationTime);

        /* Gets the current index on mKeyScalings to interpolate to based on the
        current animation time */
        int GetScaleIndex(float animationTime);

        int GetNumPositionKeys() const { return mNumPositions; }
        int GetNumRotationKeys() const { return mNumRotations; }
        int GetNumScalingKeys() const { return mNumScalings; }

        const std::vector<KeyPosition>& GetPositionKeys() const { return mPositions; }
        const std::vector<KeyRotation>& GetRotationKeys() const { return mRotations; }
        const std::vector<KeyScale>& GetScalingKeys() const { return mScales; }
        std::vector<KeyPosition>& GetPositionKeys() { return mPositions; }
        std::vector<KeyRotation>& GetRotationKeys() { return mRotations; }
        std::vector<KeyScale>& GetScalingKeys() { return mScales; }


    private:
        friend class ModelSerializer;
        friend class Animator;

        /* Gets normalized value for Lerp & Slerp*/
        float GetScaleFactor(float lastTime, float nextTime, float animationTime) const;

        /*figures out which position keys to interpolate b/w and performs the interpolation
        and returns the translation matrix*/
        void InterpolatePosition(float animationTime);

        /*figures out which rotations keys to interpolate b/w and performs the interpolation
        and returns the rotation matrix*/
        void InterpolateRotation(float animationTime);

        /*figures out which scaling keys to interpolate b/w and performs the interpolation
        and returns the scale matrix*/
        void InterpolateScaling(float animationTime);

    private:
        std::vector<KeyPosition> mPositions;
        std::vector<KeyRotation> mRotations;
        std::vector<KeyScale> mScales;
        int mNumPositions;
        int mNumRotations;
        int mNumScalings;

        glm::mat4 mLocalTransform;
        glm::mat4 mPositionMatrix;
        glm::mat4 mRotationMatrix;
        glm::mat4 mScalingMatrix;
        int mID;
    };
}
