#pragma once

#include "Mesh.h"
#include "../Texture/TextureLibrary.h"
#include "../Animation/Bone.h"

namespace Dog {

    class Texture;
    class Animation;
    class Animator;
    class UnifiedMeshes;

    class Model {
    public:
        Model(Device& device, const std::string& filePath, TextureLibrary& textureLibrary, UnifiedMeshes& unifiedMesh);
        ~Model();

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        auto& GetBoneInfoMap() { return mBoneInfoMap; }
        int& GetBoneCount() { return mBoneCounter; }

        const std::string& GetPath() const { return path; }
        bool HasAnimations() const { return hasAnimations; }
        const std::unique_ptr<Animation>& GetAnimation() const { return mAnimation; }
        const std::unique_ptr<Animator>& GetAnimator() const { return mAnimator; }

        const glm::vec3 GetModelCenter() const noexcept {
            return glm::vec3(mAnimationTransformData.x, mAnimationTransformData.y, mAnimationTransformData.z);
        }
        const float GetModelScale() const noexcept { return mAnimationTransformData.w; }

        std::vector<Mesh> meshes;


    private:
        friend class ModelSerializer;

        void loadMeshes(const std::string& filepath, TextureLibrary& textureLibrary);
        void processNode(aiNode* node, const aiScene* scene, TextureLibrary& textureLibrary, const std::string& filepath, const glm::mat4& parentTransform = glm::mat4(1.f));
        void processMesh(aiMesh* mesh, const aiScene* scene, TextureLibrary& textureLibrary, const std::string& filepath, const glm::mat4& transform);
        void processMaterials(aiMesh* mesh, const aiScene* scene, Mesh& newMesh, TextureLibrary& textureLibrary, const std::string& filepath);

        void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
        
        std::string path;
        Device& device;

        glm::vec3 mAABBmin;
        glm::vec3 mAABBmax;

        std::unordered_map<std::string, BoneInfo> mBoneInfoMap; //
        int mBoneCounter = 0;

        bool hasAnimations = false;

        // Currently a single pointer - need to figure out how multiple animations will work.
        std::unique_ptr<Animation> mAnimation;
        std::unique_ptr<Animator> mAnimator;
        glm::vec4 mAnimationTransformData; // Used for scaling to unit cube

        // Serialization
        struct SerializeData
        {
            std::vector<std::pair<uint32_t, std::unique_ptr<unsigned char[]>>> embeddedTextures;
            std::vector<std::string> mMaterialSetFilepaths;
        };

        SerializeData mSerializeData;
    };

} // namespace Dog
