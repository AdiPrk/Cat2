#pragma once

#include "ModelLoader.h"

namespace Dog
{
    class GltfLoader : public IModelLoader
    {
    public:
        bool LoadFromFile(ModelLoadData& outData, const std::string& filePath) override;

    private:
        void ProcessNode(ModelLoadData& outData, const tinygltf::Model& model, const tinygltf::Node& node, const std::string& baseDir);
        void ProcessMesh(ModelLoadData& outData, const tinygltf::Model& model, const tinygltf::Mesh& mesh, int skinIndex, const std::string& baseDir);
        void ProcessMaterial(MaterialData& outMaterial, const tinygltf::Model& model, const tinygltf::Material& material, const std::string& baseDir);

        // Helper to get a pointer and element count from a glTF accessor
        template<typename T>
        std::pair<const T*, size_t> GetAccessorData(const tinygltf::Model& model, int accessorIndex);
    };
}