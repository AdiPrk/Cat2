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
        LoadMeshes(filePath);

        // Create vertex and index buffers for all meshes
        for (Mesh& mesh : mMeshes) {
            mesh.CreateVertexBuffers(device);
            mesh.CreateIndexBuffers(device);
        }
    }

    const aiScene* Model::LoadMeshes(const std::string& filepath)
    {
        static Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(filepath, aiProcessPreset_TargetRealtime_MaxQuality);
        mScene = scene;

        // Check if the scene was loaded successfully
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            DOG_CRITICAL("Assimp Error: {}", importer.GetErrorString());
            return nullptr;
        }

        // check how many animations are in this
        if (scene->HasAnimations())
        {
            mHasAnimations = true;
        }

        mDirectory = filepath.substr(0, filepath.find_last_of('/'));
        mModelName = std::filesystem::path(filepath).stem().string();

        ProcessNode(scene->mRootNode);

        // Scale all meshes to fit in a 1x1x1 cube and translate to 0,0,0
        NormalizeModel();

        return scene;
    }

    void Model::ProcessNode(aiNode* node)
    {
        // Process each mesh in the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* aMesh = mScene->mMeshes[node->mMeshes[i]];
            Mesh& mesh = ProcessMesh(aMesh);
            ProcessMaterials(aMesh, mesh);
        }

        // Recursively process each child node
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(node->mChildren[i]);
        }
    }

    Mesh& Model::ProcessMesh(aiMesh* mesh)
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
                vertex.normal = { mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z };
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

        ExtractBoneWeightForVertices(newMesh.mVertices, mesh);

        return newMesh;
    }

    void Model::UpdateAABB(aiVector3D min, aiVector3D max)
    {
        mAABBmin = glm::min(glm::vec3(min.x, min.y, min.z), mAABBmin);
        mAABBmax = glm::max(glm::vec3(max.x, max.y, max.z), mAABBmax);
    }

    void Model::NormalizeModel()
    {
        glm::vec3 size = mAABBmax - mAABBmin;

        glm::vec3 center = (mAABBmax + mAABBmin) * 0.5f;
        float invScale = 1.f / std::max({ size.x, size.y, size.z });

        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), -center);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(invScale));
        mNormalizationMatrix = scaleMatrix * translationMatrix;
        mAnimationTransform = glm::vec4(center, invScale);
    }

    void Model::ProcessMaterials(aiMesh* mesh, Mesh& newMesh)
    {
        if (!mScene->HasMaterials()) return;

        aiMaterial* material = mScene->mMaterials[mesh->mMaterialIndex];
        ProcessBaseColor(mScene, material, newMesh);
        ProcessDiffuseTexture(mScene, material, newMesh);
    }

    void Model::ProcessBaseColor(const aiScene* mScene, aiMaterial* material, Mesh& newMesh)
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
    
    void Model::ProcessDiffuseTexture(const aiScene* mScene, aiMaterial* material, Mesh& newMesh)
    {
        aiString texturePath;
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) != AI_SUCCESS) return;

        const aiTexture* embeddedTexture = mScene->GetEmbeddedTexture(texturePath.C_Str());
        if (!embeddedTexture)
        {
            std::filesystem::path path(texturePath.C_Str());
            std::string filename = path.filename().string();

            newMesh.diffuseTexturePath = mDirectory + "/ModelTextures/" + mModelName + "/" + filename;
        }
        else if (embeddedTexture->mHeight == 0)
        {
            newMesh.mTextureSize = static_cast<uint32_t>(embeddedTexture->mWidth);

            newMesh.mTextureData = std::make_unique<unsigned char[]>(newMesh.mTextureSize);
            std::memcpy(newMesh.mTextureData.get(), embeddedTexture->pcData, static_cast<size_t>(embeddedTexture->mWidth));
        }
        else
        {
            DOG_CRITICAL("How are we here");
        }
    }

    void Model::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh)
    {
        // Iterate over all bones in the aiMesh
        for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            aiBone* bone = mesh->mBones[boneIndex];
            std::string boneName = bone->mName.C_Str();
            int boneID = -1;

            if (mBoneInfoMap.find(boneName) == mBoneInfoMap.end())
            {
                BoneInfo newBoneInfo(mBoneCounter, aiMatToGlm(bone->mOffsetMatrix));

                mBoneInfoMap[boneName] = newBoneInfo;
                boneID = mBoneCounter++;
            }
            else
            {
                boneID = mBoneInfoMap[boneName].id;
            }

            // Process each vertex weight associated with the bone
            for (unsigned int weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex)
            {
                const aiVertexWeight& weightData = bone->mWeights[weightIndex];
                int vertexId = weightData.mVertexId;
                float weight = weightData.mWeight;

                vertices[vertexId].SetBoneData(boneID, weight);
            }
        }
    }
}
