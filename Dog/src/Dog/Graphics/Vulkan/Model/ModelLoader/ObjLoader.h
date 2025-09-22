#pragma once

#include "ModelLoader.h"

namespace Dog
{
    class ObjLoader : public IModelLoader {
    public:
        // Overrides the base class method to load a model from an .obj file.
        bool LoadFromFile(ModelLoadData& outData, const std::string& filePath) override;

    private:
        // Helper to process a tinyobj material and populate our MaterialData struct.
        void ProcessMaterial(MaterialData& outMaterial, const tinyobj::material_t& material, const std::string& baseDir);
    };
}
