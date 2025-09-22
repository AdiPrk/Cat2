#include <PCH/pch.h>
#include "Model.h"
#include "Mesh.h"
#include "../Core/Buffer.h"

#include "stb_image.h"

#include "ModelLoader/ModelLoader.h"
#include "ModelLoader/GltfLoader.h"
#include "ModelLoader/ObjLoader.h"

namespace Dog
{
    // Helper function to convert tinygltf matrix to glm::mat4
    static inline glm::mat4 Mat4FromDoubleArray(const std::vector<double>& arr)
    {
        glm::mat4 m;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                m[i][j] = static_cast<float>(arr[j * 4 + i]);
            }
        }
        return m;
    }

    Model::Model(Device& device, const std::string& filePath, const ModelLoadData& loadedData)
        : device{ device }
    {
        // Transfer loaded data to the Model object
        mAABBmin = loadedData.aabbMin;
        mAABBmax = loadedData.aabbMax;
        mHasAnimations = loadedData.hasAnimations;
        mMeshes.reserve(loadedData.meshes.size());

        for (auto& meshData : loadedData.meshes)
        {
            Mesh& newMesh = mMeshes.emplace_back();
            newMesh.mVertices = meshData.vertices;
            newMesh.mVertexCount = static_cast<uint32_t>(newMesh.mVertices.size());
            newMesh.mIndices = meshData.indices;
            newMesh.mIndexCount = static_cast<uint32_t>(newMesh.mIndices.size());
            newMesh.mHasIndexBuffer = !newMesh.mIndices.empty();

            // --- Handle Material and Texture Data ---
            newMesh.diffuseTexturePath = meshData.material.diffuseTexturePath;

            if (!meshData.material.diffuseTextureData.empty())
            {
                newMesh.mTextureSize = static_cast<uint32_t>(meshData.material.diffuseTextureData.size());
                newMesh.mTextureData = std::make_unique<unsigned char[]>(newMesh.mTextureSize);
                std::memcpy(newMesh.mTextureData.get(), meshData.material.diffuseTextureData.data(), newMesh.mTextureSize);

                if (meshData.material.diffuseTextureLoaded)
                {
                    newMesh.mTextureLoaded = true;
                    newMesh.mWidth = meshData.material.diffuseWidth;
                    newMesh.mHeight = meshData.material.diffuseHeight;
                    newMesh.mChannels = 4; // Assuming RGBA
                }
            }
        }

        mDirectory = std::filesystem::path(filePath).parent_path().string();
        mModelName = std::filesystem::path(filePath).stem().string();

        // Create vertex and index buffers for all meshes
        for (Mesh& mesh : mMeshes) {
            mesh.CreateVertexBuffers(device);
            mesh.CreateIndexBuffers(device);
        }

        NormalizeModel();
    }

    Model::~Model()
    {
    }

    void Model::NormalizeModel()
    {
        glm::vec3 size = mAABBmax - mAABBmin;
        glm::vec3 center = (mAABBmax + mAABBmin) * 0.5f;
        float invScale = 1.0f / std::max({ size.x, size.y, size.z });

        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), -center);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(invScale));

        mNormalizationMatrix = scaleMatrix * translationMatrix;
        mAnimationTransform = glm::vec4(center, invScale);
    }

}
