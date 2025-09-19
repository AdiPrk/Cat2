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

        glm::vec3 GetModelCenter() const { return glm::vec3(mAnimationTransformData.x, mAnimationTransformData.y, mAnimationTransformData.z); }
        float GetModelScale() const { return mAnimationTransformData.w; }

    private:
        // Main load model function
        void LoadModel(const std::string& filePath);

        // Load and process model using assimp
        const aiScene* LoadMeshes(const std::string& filepath);
        
        void ProcessNode(aiNode* node, const aiScene* scene);
        Mesh& ProcessMesh(aiMesh* mesh, const aiScene* scene);
        
        void ProcessMaterials(aiMesh* mesh, const aiScene* scene, Mesh& newMesh);
        void ProcessBaseColor(const aiScene* scene, aiMaterial* material, Mesh& newMesh);
        void ProcessDiffuseTexture(const aiScene* scene, aiMaterial* material, Mesh& newMesh);

        void UpdateAABB(aiVector3D min, aiVector3D max);

        void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);

        Device& device; // the device!

        glm::vec3 mAABBmin;
        glm::vec3 mAABBmax;

        bool mAddedTexture = false;
        std::string mModelName;
        std::string mDirectory; // For texture loading

        // Animation data
        std::unordered_map<std::string, BoneInfo> mBoneInfoMap;
        int mBoneCounter = 0;

        bool mHasAnimations = false;
        glm::vec4 mAnimationTransformData = glm::vec4(0.f); // xyz = center, w = inv scale

        friend class AnimationLibrary;
        const aiScene* mScene = nullptr;
    };
}