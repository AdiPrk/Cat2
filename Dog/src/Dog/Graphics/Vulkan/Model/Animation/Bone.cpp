#include <PCH/pch.h>
#include "Bone.h"

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
    mNumPositions = channel->mNumPositionKeys;

    for (int positionIndex = 0; positionIndex < mNumPositions; ++positionIndex)
    {
      aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
      float time = static_cast<float>(channel->mPositionKeys[positionIndex].mTime);

      mPositions.emplace_back(aiVecToGlm(aiPosition), time);
    }

    mNumRotations = channel->mNumRotationKeys;
    for (int rotationIndex = 0; rotationIndex < mNumRotations; ++rotationIndex)
    {
      aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
      float time = static_cast<float>(channel->mRotationKeys[rotationIndex].mTime);

      mRotations.emplace_back(aiQuatToGlm(aiOrientation), time);
    }

    mNumScalings = channel->mNumScalingKeys;

    for (int keyIndex = 0; keyIndex < mNumScalings; ++keyIndex)
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

  int Bone::GetPositionIndex(float animationTime)
  {
    for (int index = 0; index < mNumPositions - 1; ++index)
    {
      if (animationTime < mPositions[index + 1].time) return index;
    }
    
    return 0;
  }

  int Bone::GetRotationIndex(float animationTime)
  {
    for (int index = 0; index < mNumRotations - 1; ++index)
    {
      if (animationTime < mRotations[index + 1].time) return index;
    }

    return 0;
  }
  
  int Bone::GetScaleIndex(float animationTime)
  {
    for (int index = 0; index < mNumScalings - 1; ++index)
    {
      if (animationTime < mScales[index + 1].time) return index;
    }
    
    return 0;
  }

  float Bone::GetScaleFactor(float lastTime, float nextTime, float animationTime) const
  {
    float midWayLength = animationTime - lastTime;
    float framesDiff = nextTime - lastTime;
    // CARE 0
    return midWayLength / framesDiff;
  }

  void Bone::InterpolatePosition(float animationTime)
  {
    if (mNumPositions == 1)
    {
      mPositionMatrix = glm::translate(glm::mat4(1.0f), mPositions[0].position);
      return;
    }

    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;

    float scaleFactor = GetScaleFactor(mPositions[p0Index].time, mPositions[p1Index].time, animationTime);
    glm::vec3 finalPosition = glm::mix(mPositions[p0Index].position, mPositions[p1Index].position, scaleFactor);

    mPositionMatrix = glm::translate(glm::mat4(1.0f), finalPosition);
  }

  void Bone::InterpolateRotation(float animationTime)
  {
    if (mNumRotations == 1)
    {
      mRotationMatrix = glm::toMat4(glm::normalize(mRotations[0].orientation));
      return;
    }

    int p0Index = GetRotationIndex(animationTime);
    int p1Index = p0Index + 1;

    float scaleFactor = GetScaleFactor(mRotations[p0Index].time, mRotations[p1Index].time, animationTime);
    glm::quat finalRotation = glm::slerp(mRotations[p0Index].orientation, mRotations[p1Index].orientation, scaleFactor);

    mRotationMatrix = glm::toMat4(glm::normalize(finalRotation));
  }

  void Bone::InterpolateScaling(float animationTime)
  {
    if (mNumScalings == 1)
    {
      mScalingMatrix = glm::scale(glm::mat4(1.0f), mScales[0].scale);
      return;
    }

    int p0Index = GetScaleIndex(animationTime);
    int p1Index = p0Index + 1;

    float scaleFactor = GetScaleFactor(mScales[p0Index].time, mScales[p1Index].time, animationTime);
    glm::vec3 finalScale = glm::mix(mScales[p0Index].scale, mScales[p1Index].scale, scaleFactor);

    mScalingMatrix = glm::scale(glm::mat4(1.0f), finalScale);
  }
}