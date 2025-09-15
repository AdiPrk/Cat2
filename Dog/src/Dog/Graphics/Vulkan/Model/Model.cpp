#include <PCH/pch.h>
#include "Model.h"
#include "Mesh.h"
#include "../Core/Buffer.h"

namespace Dog
{
    Model::Model(Device& device, const std::string& filePath)
        : device{ device }
    {
        LoadModel(filePath);
    }

    Model::~Model()
    {
    }

    void Model::LoadModel(const std::string& filePath)
    {
        const aiScene* scene = LoadMeshes(filePath);

        // Create vertex and index buffers for all meshes
        for (Mesh& mesh : mMeshes) {
            mesh.CreateVertexBuffers(device);
            mesh.CreateIndexBuffers(device);
        }
    }

    const aiScene* Model::LoadMeshes(const std::string& filepath)
    {
        static Assimp::Importer importer;

        unsigned int processFlags = aiProcessPreset_TargetRealtime_MaxQuality;
        processFlags |= aiProcess_GlobalScale;

        const aiScene* scene = importer.ReadFile(filepath, 0);

        // Check if the scene was loaded successfully
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            DOG_CRITICAL("Assimp Error: {}", importer.GetErrorString());
            return nullptr;
        }

        mDirectory = filepath.substr(0, filepath.find_last_of('/'));
        ProcessNode(scene->mRootNode, scene);

        return scene;
    }

    void Model::ProcessNode(aiNode* node, const aiScene* scene)
    {
        // Process each mesh in the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];
            Mesh& mesh = ProcessMesh(aMesh, scene);
            ProcessMaterials(aMesh, scene, mesh);
        }

        // Recursively process each child node
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(node->mChildren[i], scene);
        }
    }

    Mesh& Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
    {
        Mesh& newMesh = mMeshes.emplace_back();
        
        aiVector3D min(FLT_MAX, FLT_MAX, FLT_MAX);
        aiVector3D max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        // Extract vertex data
        for (unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            Vertex vertex{};

            vertex.position = { mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z };

            // Update min and max vectors for AABB
            min.x = std::min(min.x, vertex.position.x);
            min.y = std::min(min.y, vertex.position.y);
            min.z = std::min(min.z, vertex.position.z);
            max.x = std::max(max.x, vertex.position.x);
            max.y = std::max(max.y, vertex.position.y);
            max.z = std::max(max.z, vertex.position.z);

            // Normals
            if (mesh->HasNormals())
            {
                glm::vec3 normal = {
                    mesh->mNormals[j].x,
                    mesh->mNormals[j].y,
                    mesh->mNormals[j].z
                };

                vertex.normal = normal;
            }

            // UV Coordinates
            if (mesh->HasTextureCoords(0))
            {
                vertex.uv = { mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y };
            }

            // Colors
            if (mesh->HasVertexColors(0))
            {
                vertex.color = { mesh->mColors[0][j].r, mesh->mColors[0][j].g, mesh->mColors[0][j].b };
            }

            newMesh.mVertices.push_back(vertex);
        }

        //Update model's AABB
        UpdateAABB(min, max);

        // Extract indices from faces
        for (unsigned int k = 0; k < mesh->mNumFaces; k++)
        {
            const aiFace& face = mesh->mFaces[k];
            for (unsigned int l = 0; l < face.mNumIndices; l++)
            {
                newMesh.mIndices.push_back(face.mIndices[l]);
            }
        }

        return newMesh;
    }

    void Model::UpdateAABB(aiVector3D min, aiVector3D max)
    {
        mAABBmin = glm::min(glm::vec3(min.x, min.y, min.z), mAABBmin);
        mAABBmax = glm::max(glm::vec3(max.x, max.y, max.z), mAABBmax);
    }

    void Model::ProcessMaterials(aiMesh* mesh, const aiScene* scene, Mesh& newMesh)
    {
        if (!scene->HasMaterials()) return;

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        ProcessBaseColor(scene, material, newMesh);
        ProcessDiffuseTexture(scene, material, newMesh);
    }

    void Model::ProcessBaseColor(const aiScene* scene, aiMaterial* material, Mesh& newMesh)
    {
        bool hasBaseColor = false;
        aiColor4D baseColor;
        if (material->Get(AI_MATKEY_BASE_COLOR, baseColor) == AI_SUCCESS) {
            hasBaseColor = true;
        }
        else if (material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor) == AI_SUCCESS) {
            hasBaseColor = true;
        }
        if (hasBaseColor)
        {
            for (Vertex& vertex : newMesh.mVertices)
            {
                vertex.color *= glm::vec3(baseColor.r, baseColor.g, baseColor.b);
            }
        }
    }
    
    void Model::ProcessDiffuseTexture(const aiScene* scene, aiMaterial* material, Mesh& newMesh)
    {
        aiString texturePath;
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) != AI_SUCCESS) return;

        const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(texturePath.C_Str());
        if (!embeddedTexture)
        {
            std::filesystem::path path(texturePath.C_Str());
            std::string filename = path.filename().string();

            newMesh.diffuseTexturePath = mDirectory + "/ModelTextures/" + filename;
        }
        else if (embeddedTexture->mHeight == 0)
        {

        }
        else
        {
            DOG_CRITICAL("How are we here");
        }
    }
}
