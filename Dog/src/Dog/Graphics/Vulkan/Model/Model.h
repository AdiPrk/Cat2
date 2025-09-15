#pragma once

#include "Mesh.h"

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

        Device& device; // the device!

        glm::vec3 mAABBmin;
        glm::vec3 mAABBmax;

        bool mAddedTexture = false;
        std::string mDirectory; // For texture loading
    };
}