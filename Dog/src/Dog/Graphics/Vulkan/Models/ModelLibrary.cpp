#include <PCH/pch.h>
#include "ModelLibrary.h"
#include "Model.h"
#include "../Core/Device.h"
#include "../Texture/TextureLibrary.h"

namespace Dog {

	ModelLibrary::ModelLibrary(Device& device, TextureLibrary& textureLibrary)
		: m_Device(device)
		, m_TextureLibrary(textureLibrary)
	{
	}

	ModelLibrary::~ModelLibrary()
	{
	}

	uint32_t ModelLibrary::AddModel(const std::string& modelPath)
	{
		if (m_Models.size() >= MAX_MODEL_COUNT) {
			throw std::runtime_error("Model count exceeded maximum");
			return INVALID_MODEL_INDEX;
		}

		// Cut off everything before the last slash and cut off extension
		std::string cutPath = modelPath;
		size_t lastSlash = modelPath.find_last_of("/\\");
		if (lastSlash != std::string::npos)
		{
			cutPath = modelPath.substr(lastSlash + 1);
		}

		size_t lastDot = cutPath.find_last_of(".");
		if (lastDot != std::string::npos)
		{
			cutPath = cutPath.substr(0, lastDot);
		}

		if (m_ModelMap.find(modelPath) == m_ModelMap.end()) 
		{
			uint32_t modelIndex = static_cast<uint32_t>(m_Models.size());

			m_ModelMap[modelPath] = modelIndex;
			m_Models.push_back(std::make_unique<Model>(m_Device, modelPath, m_TextureLibrary, mUnifiedMesh));
			return static_cast<uint32_t>(modelIndex);
		}
		else 
		{
			return m_ModelMap[modelPath];
		}
	}

	uint32_t ModelLibrary::GetModel(const std::string& modelPath)
	{
		if (m_ModelMap.find(modelPath) != m_ModelMap.end()) {
			return m_ModelMap[modelPath];
		}
		else {
			// try adding it
			return AddModel(modelPath);
		}
	}

	Model* ModelLibrary::GetModelByIndex(uint32_t index)
	{
		if (index == INVALID_MODEL_INDEX) return nullptr;

		if (index < m_Models.size()) {
			return m_Models[index].get();
		}
		else {
			return nullptr;
		}
	}

} // namespace Dog