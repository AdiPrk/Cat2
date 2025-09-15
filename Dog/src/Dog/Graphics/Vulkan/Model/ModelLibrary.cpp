#include <PCH/pch.h>
#include "ModelLibrary.h"
#include "Model.h"

#include "../Texture/TextureLibrary.h"

namespace Dog
{
    const uint32_t ModelLibrary::INVALID_MODEL_INDEX = 9999;

    ModelLibrary::ModelLibrary(Device& device, TextureLibrary& textureLibrary)
        : mDevice{ device }
        , mTextureLibrary{ textureLibrary }
    {
    }

    ModelLibrary::~ModelLibrary()
    {
        mModels.clear();
    }

    uint32_t ModelLibrary::AddModel(const std::string& filePath)
    {
        auto it = mModelMap.find(filePath);
        if (it != mModelMap.end())
        {
            return it->second;
        }

        std::unique_ptr<Model> model = std::make_unique<Model>(mDevice, filePath);
        
        uint32_t modelID = static_cast<uint32_t>(mModels.size());
        mModels.push_back(std::move(model));
        mModelMap[filePath] = modelID;
        
        return modelID;
    }

    Model* ModelLibrary::GetModel(uint32_t index)
    {
        if (index >= mModels.size())
        {
            DOG_CRITICAL("Model ID {0} is out of range!", index);
            return nullptr;
        }

        return mModels[index].get();
    }

    Model* ModelLibrary::GetModel(const std::string& modelPath)
    {
        auto it = mModelMap.find(modelPath);
        if (it == mModelMap.end())
        {
            DOG_CRITICAL("Model path {0} not found in library!", modelPath);
            return nullptr;
        }
        return GetModel(it->second);
    }

    uint32_t ModelLibrary::GetModelIndex(const std::string& modelPath)
    {
        auto it = mModelMap.find(modelPath);
        if (it == mModelMap.end())
        {
            return INVALID_MODEL_INDEX;
        }

        return it->second;
    }

    void ModelLibrary::LoadTextures()
    {
        for (auto& model : mModels)
        {
            for (auto& mesh : model->mMeshes)
            {
                if (mesh.diffuseTexturePath.empty()) continue;
                
                uint32_t ind = mTextureLibrary.AddTexture(mesh.diffuseTexturePath);
                mesh.diffuseTextureIndex = ind;
            }
        }
    }
}
