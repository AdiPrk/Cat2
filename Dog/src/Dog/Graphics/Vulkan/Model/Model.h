#pragma once

#include "Mesh.h"
#include "Animation/Bone.h"

namespace Dog
{
    class Device;

    class Model
    {
    public:
        //Remove copy constructor/operation from class 
        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        Model(Device& device, const std::string& filePath);
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

        // Load and process model using assimp
        void LoadMeshes(const std::string& filepath);
        
        void ProcessNode(aiNode* node);
        Mesh& ProcessMesh(aiMesh* mesh);
        
        void ProcessMaterials(aiMesh* mesh, Mesh& newMesh);
        void ProcessBaseColor(const aiScene* scene, aiMaterial* material, Mesh& newMesh);
        void ProcessDiffuseTexture(const aiScene* scene, aiMaterial* material, Mesh& newMesh);

        void NormalizeModel();

        void ExtractBoneWeights(std::vector<Vertex>& vertices, aiMesh* mesh);

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

        friend class AnimationLibrary;
        const aiScene* mScene = nullptr;
    };
}