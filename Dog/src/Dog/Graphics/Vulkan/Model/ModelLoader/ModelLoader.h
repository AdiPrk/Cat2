#pragma once

#include "../Vertex.h"

namespace Dog
{
    // All the data required to load materials for a mesh
    struct MaterialData
    {
        std::string diffuseTexturePath;                // Path to the texture file if loaded from disk
        std::vector<unsigned char> diffuseTextureData; // Compressed or uncompressed pixel data depending on diffuseTextureLoaded
        bool diffuseTextureLoaded = false;             // If the texture is already loaded
        int diffuseWidth{ 0 };                         // If the texture is already loaded
        int diffuseHeight{ 0 };                        // If the texture is already loaded

    };

    // All the data for a single mesh
    struct MeshData
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        MaterialData material;
    };

    // All the data required for a model
    struct ModelLoadData
    {
        std::vector<MeshData> meshes;
        glm::vec3 aabbMin{ std::numeric_limits<float>::max() };
        glm::vec3 aabbMax{ std::numeric_limits<float>::lowest() };
        bool hasAnimations = false;
    };

    // Abstract base class for all model loaders
    class IModelLoader {
    public:
        virtual ~IModelLoader() = default;
        virtual bool LoadFromFile(ModelLoadData& outData, const std::string& filePath) = 0;
    };

    // The main model loader that delegates to specific format loaders
    class ModelLoader
    {
    public:
        ModelLoader();
        ~ModelLoader() {}

        bool LoadModel(ModelLoadData& outData, const std::string& filePath);

    private:
        std::unordered_map<std::string, std::unique_ptr<IModelLoader>> mLoaders;
    };
}