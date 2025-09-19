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

    void Update(float animationTime);

    inline const glm::mat4& GetLocalTransform() const noexcept { return mLocalTransform; }

    int GetBoneID() const { return mID; }

    const std::vector<KeyPosition>& GetPositionKeys() const { return mPositions; }
    const std::vector<KeyRotation>& GetRotationKeys() const { return mRotations; }
    const std::vector<KeyScale>& GetScalingKeys()     const { return mScales;    }

  private:
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

    glm::mat4 mLocalTransform;
    glm::mat4 mPositionMatrix;
    glm::mat4 mRotationMatrix;
    glm::mat4 mScalingMatrix;
    int mID;
  };
}
