#include <PCH/pch.h>
#include "Model.h"
#include "../Animation/Animator.h"
#include "../Texture/Texture.h"

#include "assimp/Exporter.hpp"
#include "UnifiedMeshes.h"

namespace Dog {

    Model::Model(Device& device, const std::string& filePath, TextureLibrary& textureLibrary, UnifiedMeshes& unifiedMesh)
        : device{ device }
        , path(filePath)
        , mAABBmin(0.0f)
        , mAABBmax(0.0f)
    {
        loadMeshes(filePath, textureLibrary);

        for (Mesh& mesh : meshes) {
            mesh.createVertexBuffers(device);
            mesh.createIndexBuffers(device);
            unifiedMesh.AddMesh(device, mesh);
        }
    }

    Model::~Model() {}

    std::string aiTexturePathToNLEPath(const aiString& texturePath) {
        std::string textureFilepath = texturePath.C_Str();

        textureFilepath.erase(0, textureFilepath.find_first_not_of("./\\"));
        textureFilepath.erase(0, textureFilepath.find_last_of("/\\") + 1);

        std::string textureFullpath = "assets/models/ModelTextures/" + textureFilepath;
 
        return textureFullpath;
    }

    // aiProcess_Triangulate              // Ensure all faces are triangles
    // | aiProcess_JoinIdenticalVertices  // Combine identical vertices
    // | aiProcess_GenNormals             // Generate normals if they don't exist
    // | aiProcess_LimitBoneWeights       // Limit bone influences (relevant for animated FBX)
    // | aiProcess_OptimizeGraph		   // Optimize the scene graph (Bad if we want a cool editor doing stuff, good for fast rendering)
    // | aiProcess_OptimizeMeshes         // Optimize the meshes for better performance
    // | aiProcess_SplitLargeMeshes       // Split large meshes into smaller submeshes
    // | aiProcess_RemoveRedundantMaterials // Remove redundant materials (be careful)
    // | aiProcess_ImproveCacheLocality   // Improve GPU cache performance

    void Model::loadMeshes(const std::string& filepath, TextureLibrary& textureLibrary) {
        // Making an Importer is supposedly expensive, so I made it static
        static Assimp::Importer importer;

        // Log the file being loaded
        std::cout << "Loading model: " << filepath << std::endl;

        try {
            // Not so sure about these flags
            // Maybe we can pre-load models and store them somewhere
            // so that in the real game it doesn't have to read the model slowly?
            unsigned int processFlags = aiProcessPreset_TargetRealtime_Quality; // slowest to load, but highest quality

            // Since lines and points aren't supported right now
            importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

            // set global scale
            importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.f);
            processFlags |= aiProcess_GlobalScale;

            // cut filepath until the last slash and remove extension
            std::string filename = filepath.substr(filepath.find_last_of("/\\") + 1);
            filename = filename.substr(0, filename.find_last_of("."));
            std::string assbinFilename = "assets/models/cached/" + filename + ".assbin";

            // check if the assbin file exists
            bool doesAssbinExist = false;
            {
                std::ifstream assbinFileCheck(assbinFilename);
                if (!assbinFileCheck.good()) doesAssbinExist = false;
            }

            const aiScene* scene;
            if (doesAssbinExist) {
                scene = importer.ReadFile(assbinFilename, 0);
            }
            else {
                scene = importer.ReadFile(filepath, processFlags);
                if (scene) {
                    aiReturn result = aiExportScene(scene, "assbin", assbinFilename.c_str(), 0);
                }
            }

            // Check if the scene was loaded successfully
            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cerr << "Error loading model: " << importer.GetErrorString() << std::endl;
                throw std::runtime_error("Failed to load model: " + std::string(importer.GetErrorString()));
            }

            meshes.clear();

            // Start recursive loading all the meshes
            //glm::mat4 globalTransform = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));  // Start with identity matrix
            //processNode(scene->mRootNode, scene, textureLibrary, filepath, globalTransform);
            processNode(scene->mRootNode, scene, textureLibrary, filepath);

            // Log model loaded successfully with number of verts and indices
            int numVertices = 0;
            int numIndices = 0;

            for (const Mesh& mesh : meshes) {
                numVertices += int(mesh.vertices.size());
                numIndices += int(mesh.indices.size());
            }

            DOG_INFO("Model loaded successfully: {0} ({1} meshes, {2} vertices, {3} indices)", filepath, meshes.size(), numVertices, numIndices);
        }
        catch (const std::exception& e) {
            std::cerr << "Exception occurred while loading model: " << e.what() << std::endl;
            throw;  // Rethrow the exception after logging
        }
    }

    // Recursive function to process a node and its children
    void Model::processNode(aiNode* node, const aiScene* scene, TextureLibrary& textureLibrary, const std::string& filepath, const glm::mat4& parentTransform) {
        // Convert the node's transformation matrix
        glm::mat4 nodeTransform = aiMatToGlm(node->mTransformation);

        // Combine this node's transformation with the parent transformation
        glm::mat4 globalTransform = parentTransform * nodeTransform;

        // Process each mesh in the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            processMesh(mesh, scene, textureLibrary, filepath, globalTransform);
        }

        // Recursively process each child node
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene, textureLibrary, filepath, globalTransform);
        }
    }

    // Process a single mesh and extract vertices, indices, and materials
    void Model::processMesh(aiMesh* mesh, const aiScene* scene, TextureLibrary& textureLibrary, const std::string& filepath, const glm::mat4& transform) {
        Mesh& newMesh = meshes.emplace_back();
        newMesh.vertices.clear();
        newMesh.indices.clear();

        // printf("-  New Mesh\n");

        // Extract vertex data
        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            Vertex vertex{};

            glm::vec4 position = {
                mesh->mVertices[j].x,
                mesh->mVertices[j].y,
                mesh->mVertices[j].z,
                1.0f
            };
            position = transform * position;

            vertex.position = glm::vec3(position.x, position.y, position.z);

            // Normals
            if (mesh->HasNormals()) {
              glm::vec3 normal = {
                  mesh->mNormals[j].x,
                  mesh->mNormals[j].y,
                  mesh->mNormals[j].z
              };

              // Apply transformation to the normal
              glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
              vertex.normal = normalMatrix * normal;
            }
            else {
                vertex.normal = glm::vec3(0.0f);
            }

            // UV Coordinates
            if (mesh->HasTextureCoords(0)) {
                vertex.uv = {
                    mesh->mTextureCoords[0][j].x,
                    mesh->mTextureCoords[0][j].y
                };
            }
            else {
                vertex.uv = glm::vec2(0.0f);
            }

            // Colors
            if (mesh->HasVertexColors(0)) {
                vertex.color = {
                    mesh->mColors[0][j].r,
                    mesh->mColors[0][j].g,
                    mesh->mColors[0][j].b
                };
            }
            else {
                vertex.color = glm::vec3(1.0f); // Default white color
            }

            newMesh.vertices.push_back(vertex);
        }

        // Process materials and textures
        processMaterials(mesh, scene, newMesh, textureLibrary, filepath);

        ExtractBoneWeightForVertices(newMesh.vertices, mesh, scene);

        // Extract indices from faces
        for (unsigned int k = 0; k < mesh->mNumFaces; k++) {
            const aiFace& face = mesh->mFaces[k];
            for (unsigned int l = 0; l < face.mNumIndices; l++) {
                newMesh.indices.push_back(face.mIndices[l]);
            }
        }
    }

    // process materials
    void Model::processMaterials(aiMesh* mesh, const aiScene* scene, Mesh& newMesh, TextureLibrary& textureLibrary, const std::string& filepath) {
        // Loop through materials (textures)
        if (scene->HasMaterials()) {
            // auto& newMaterial = newMesh.material;
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            aiString texturePath;

            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
              scene->GetEmbeddedTexture(texturePath.C_Str());

                if (texturePath.data[0] == '*') {
                    int textureIndex = atoi(texturePath.C_Str() + 1);

                    if (textureIndex >= 0 && textureIndex < int(scene->mNumTextures)) {
                        aiTexture* embeddedTexture = scene->mTextures[textureIndex];

                        if (embeddedTexture->mHeight == 0) {
                            unsigned char* textureData = reinterpret_cast<unsigned char*>(embeddedTexture->pcData);
                            int textureSize = embeddedTexture->mWidth;

                            newMesh.materialComponent.AlbedoTexture = textureLibrary.AddTextureFromMemory(textureData, textureSize);
                        }
                    }
                }
                else {
                    std::string textureFullpath = aiTexturePathToNLEPath(texturePath);

                    textureLibrary.AddTexture(textureFullpath);
                    newMesh.materialComponent.AlbedoTexture = textureLibrary.GetTexture(textureFullpath);
                }
            }
        }
    }

    void Model::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
    {
        auto& boneInfoMap = mBoneInfoMap;
        int& boneCount = mBoneCounter;

        for (unsigned boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            int boneID = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            if (boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                BoneInfo newBoneInfo;
                newBoneInfo.id = boneCount;
                newBoneInfo.offset = aiMatToGlm(mesh->mBones[boneIndex]->mOffsetMatrix);
                boneInfoMap[boneName] = newBoneInfo;
                boneID = boneCount;
                boneCount++;
            }
            else
            {
                boneID = boneInfoMap[boneName].id;
            }
            
            auto weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;

                vertices[vertexId].SetBoneData(boneID, weight);
            }
        }
    }

} // namespace Dog
