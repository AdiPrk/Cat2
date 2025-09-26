/*****************************************************************//**
 * \file   ModelLibrary.hpp
 * \author Adi (aditya.prakash@digipen.edu)
 * \date   September 26 2024
 * \Copyright @ 2024 Digipen (USA) Corporation *

 * \brief  Library for models
 *  *********************************************************************/
#pragma once

#include "../Core/Device.h"

namespace Dog
{
	class Model;
	class Uniform;
	class TextureLibrary;

	class ModelLibrary
	{
	public:
		ModelLibrary(Device& device, TextureLibrary& textureLibrary);
		~ModelLibrary();

		uint32_t AddModel(const std::string& modelPath);

        Model* GetModel(uint32_t index);
        Model* GetModel(const std::string& modelPath);
		uint32_t GetModelIndex(const std::string& modelPath);

        uint32_t GetModelCount() const { return static_cast<uint32_t>(mModels.size()); }
		
		void LoadTextures();
		bool NeedsTextureUpdate();
        const static uint32_t INVALID_MODEL_INDEX;

	private:
		friend class Model;

		std::vector<std::unique_ptr<Model>> mModels;
		std::unordered_map<std::string, uint32_t> mModelMap;

		Device& mDevice;
        TextureLibrary& mTextureLibrary;

        uint32_t mLastModelLoaded = INVALID_MODEL_INDEX;
	};

} // namespace Rendering