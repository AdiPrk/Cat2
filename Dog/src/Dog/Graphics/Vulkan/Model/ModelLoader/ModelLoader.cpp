#include <PCH/pch.h>
#include "ModelLoader.h"

#include "GltfLoader.h"
#include "ObjLoader.h"

namespace Dog
{
    
    ModelLoader::ModelLoader()
    {
        // Register loaders for different file formats
        mLoaders[".gltf"] = std::make_unique<GltfLoader>();
        mLoaders[".glb"] = std::make_unique<GltfLoader>();
        mLoaders[".obj"] = std::make_unique<ObjLoader>();
    }

    bool ModelLoader::LoadModel(ModelLoadData& outData, const std::string& filePath)
    {
        std::string extension = std::filesystem::path(filePath).extension().string();

        auto it = mLoaders.find(extension);
        if (it == mLoaders.end()) {
            DOG_CRITICAL("Unsupported model file format: {}", extension);
            return false;
        }
        
        return it->second->LoadFromFile(outData, filePath);
    }

}