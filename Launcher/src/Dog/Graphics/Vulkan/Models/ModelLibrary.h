#pragma once

#include "UnifiedMeshes.h"

namespace Dog {

	class Model;
	class Device;
	class TextureLibrary;

	class ModelLibrary
	{
	public:
		ModelLibrary(Device& device, TextureLibrary& textureLibrary);
		~ModelLibrary();

		const static uint32_t MAX_MODEL_COUNT = 100;
		const static uint32_t INVALID_MODEL_INDEX = 9999;

		/*********************************************************************
		 * param:  modelPath: path to the model file
		 * return: index of the model in the library
		 *
		 * brief:  Adds a model to the library if it doesn't already exist.
		 *********************************************************************/
		uint32_t AddModel(const std::string& modelPath);

		/*********************************************************************
		 * param:  modelPath: path to the model file
		 * return: index of the model in the library
		 *
		 * brief: Gets the index of the model in the library.
		 *********************************************************************/
		uint32_t GetModel(const std::string& modelPath);

		/*********************************************************************
		 * param:  index: The model index
		 * return: The model at the index
		 *
		 * brief:  Gets the model at the given index
		 *********************************************************************/
		Model* GetModelByIndex(uint32_t index);

		/*********************************************************************
		 * return: The number of models in the library
		 * 
		 * brief: Get the number of models in the library
		 *********************************************************************/
		uint32_t GetModelCount() const { return static_cast<uint32_t>(m_Models.size()); }

	private:
		std::vector<std::unique_ptr<Model>> m_Models;
		std::unordered_map<std::string, uint32_t> m_ModelMap;

		Device& mDevice;
		TextureLibrary& mTextureLibrary;

		UnifiedMeshes mUnifiedMesh;
	};

} // namespace Dog
