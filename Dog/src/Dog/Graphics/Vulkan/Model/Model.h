#pragma once

#include "Mesh.h"
#include "Animation/Bone.h"
#include "ModelLoader/ModelLoader.h"

namespace Dog
{
    class Device;

    class Model
    {
    public:
        //Remove copy constructor/operation from class 
        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        Model(Device& device, const std::string& filePath, const ModelLoadData& loadedData);
        ~Model();

        std::vector<Mesh> mMeshes;

        std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap() { return mBoneInfoMap; }
        const std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap() const { return mBoneInfoMap; }
        int& GetBoneCount() { return mBoneCounter; }
        bool HasAnimations() const { return mHasAnimations; }

        glm::vec3 GetModelCenter() const { return glm::vec3(mAnimationTransform.x, mAnimationTransform.y, mAnimationTransform.z); }
        float GetModelScale() const { return mAnimationTransform.w; }

    private:
        // Main load model function
        void LoadModel(const std::string& filePath);
        void NormalizeModel();

        void ExtractBoneWeights(const tinygltf::Model& model, const tinygltf::Primitive& primitive, int skinIndex, std::vector<Vertex>& vertices) {}


        Device& device; // the device!

        glm::vec3 mAABBmin;
        glm::vec3 mAABBmax;

        bool mAddedTexture = false;
        std::string mModelName;
        std::string mDirectory; // For texture loading

        friend class RenderSystem;
        glm::mat4 mNormalizationMatrix;

        // Animation data
        std::unordered_map<std::string, BoneInfo> mBoneInfoMap;
        int mBoneCounter = 0;

        bool mHasAnimations = false;
        glm::vec4 mAnimationTransform = glm::vec4(0.f); // xyz = center, w = inv scale
    };
}