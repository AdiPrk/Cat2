#include <PCH/pch.h>
#include "sceneSerializer.h"
#include "conversions.h"
#include "../scene.h"
#include "../Entity/entity.h"
#include "../Entity/components.h"
#include "Engine.h"
//#include "Dog/Assets/Packer/assetPacker.h"

/*

struct CameraComponent
	{
		// The camera that is currently being used to render the scene
		bool MainCamera = true;

		enum class CameraType
		{
			Orthographic = 0,
			Perspective
		};

		CameraType Projection = CameraType::Perspective;

		float OrthographicLeft = -1.0f;
		float OrthographicRight = 1.0f;
		float OrthographicBottom = 1.0f;
		float OrthographicTop = -1.0f;
		float OrthographicNear = -1.0f;
		float OrthographicFar = 1.0f;

		float PerspectiveFOV = 45.0f;
		float PerspectiveNear = 0.01f;
		float PerspectiveFar = 1000.0f;
*/

namespace Dog {

	void SceneSerializer::Serialize(Scene* scene, const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << scene->GetName();

		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		entt::registry& registry = scene->GetRegistry();
		std::vector<entt::entity> entities;

		registry.view<TagComponent>().each([&](const auto& entityID, const TagComponent& tagComponent)
		{
			entities.push_back(entityID);
		});

		// Reverse the vector so the order is the same when deserialized
		std::reverse(entities.begin(), entities.end());
		for (const auto& entityID : entities)
		{
			Entity entity(scene, entityID);
			SerializeEntity(out, &entity);
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::Deserialize(Scene* scene, const std::string& filepath)
	{
#ifndef DOG_SHIP
		YAML::Node data = YAML::LoadFile(filepath);
#else
		const std::string& sceneDataStr = DogFilePacker::getSceneYAMLString(filepath);
		YAML::Node data = YAML::Load(sceneDataStr);
#endif

		// check if file loaded
		if (!data)
		{
			DOG_ERROR("SceneSerializer::Deserialize: Scene {} not found", filepath);
			return;
		}

		if (!data["Scene"])
		{
			DOG_ERROR("SceneSerializer::Deserialize: Scene file does not contain a Scene tag");
			return;
		}

		scene->ClearEntities();

		std::string sceneName = data["Scene"].as<std::string>();

		YAML::Node entities = data["Entities"];

		if (entities)
		{
			for (auto entity : entities)
			{
				std::string name = entity["Entity"].as<std::string>();
				DOG_TRACE("Deserializing entity: {0}", name);

				Entity deserializedEntity = scene->CreateEmptyEntity(name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					glm::vec3 translation = transformComponent["Translation"].as<glm::vec3>();
					glm::vec3 rotation = transformComponent["Rotation"].as<glm::vec3>();
					glm::vec3 scale = transformComponent["Scale"].as<glm::vec3>();
					deserializedEntity.AddComponent<TransformComponent>(translation, rotation, scale);
				}

				auto modelComponent = entity["ModelComponent"];
				if (modelComponent)
				{
					std::string modelPath = modelComponent["ModelPath"].as<std::string>();
					auto& model = deserializedEntity.AddComponent<ModelComponent>(modelPath);
					
					auto materials = modelComponent["Materials"];
					if (materials)
					{
						model.MaterialOverrides.clear();

						for (auto material : materials)
						{
							std::string albedoTexture = material["AlbedoTexture"].as<std::string>();
							std::string normalTexture = material["NormalTexture"].as<std::string>();

							int albedoIndex = INVALID_TEXTURE_INDEX;
							int normalIndex = INVALID_TEXTURE_INDEX;

							// get texture index from path
							auto& txl = Engine::Get().GetTextureLibrary();

							if (albedoTexture != "None") {
								albedoIndex = (int)txl.GetTexture(albedoTexture);
							}
							if (normalTexture != "None") {
								normalIndex = (int)txl.GetTexture(normalTexture);
							}

							model.MaterialOverrides.push_back({ albedoIndex, normalIndex });
						}
					}
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent) {
					deserializedEntity.AddComponent<CameraComponent>();

					auto& cc = deserializedEntity.GetComponent<CameraComponent>();
					cc.MainCamera = cameraComponent["MainCamera"].as<bool>();
					cc.Projection = (CameraComponent::CameraType)cameraComponent["Projection"].as<int>();
					cc.OrthographicLeft = cameraComponent["OrthographicLeft"].as<float>();
					cc.OrthographicRight = cameraComponent["OrthographicRight"].as<float>();
					cc.OrthographicBottom = cameraComponent["OrthographicBottom"].as<float>();
					cc.OrthographicTop = cameraComponent["OrthographicTop"].as<float>();
					cc.OrthographicNear = cameraComponent["OrthographicNear"].as<float>();
					cc.OrthographicFar = cameraComponent["OrthographicFar"].as<float>();
					cc.PerspectiveFOV = cameraComponent["PerspectiveFOV"].as<float>();
					cc.PerspectiveNear = cameraComponent["PerspectiveNear"].as<float>();
					cc.PerspectiveFar = cameraComponent["PerspectiveFar"].as<float>();
				}
			}
		}
	}

	void SceneSerializer::SerializeEntity(YAML::Emitter& out, Entity* entity)
	{
		if (!entity->HasComponent<TagComponent>()) return;

		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << entity->GetComponent<TagComponent>().Tag;

		if (entity->HasComponent<TransformComponent>())
		{
			auto& tc = entity->GetComponent<TransformComponent>();

			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;
			out << YAML::EndMap;
		}

		if (entity->HasComponent<ModelComponent>())
		{
			auto& mc = entity->GetComponent<ModelComponent>();

			out << YAML::Key << "ModelComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "ModelPath" << YAML::Value << mc.ModelPath;
			out << YAML::Key << "Materials" << YAML::Value << YAML::BeginSeq;
			for (auto& material : mc.MaterialOverrides)
			{ 
				out << YAML::BeginMap;
				// get texture path from index
				auto& txl = Engine::Get().GetTextureLibrary();
				if (material.AlbedoTexture == INVALID_TEXTURE_INDEX) {
					out << YAML::Key << "AlbedoTexture" << YAML::Value << "None";
				}
				else {
					std::string& albedoPath = txl.getTextureByIndex(material.AlbedoTexture).path;
					out << YAML::Key << "AlbedoTexture" << YAML::Value << albedoPath;
				}

				if (material.NormalTexture == INVALID_TEXTURE_INDEX) {
					out << YAML::Key << "NormalTexture" << YAML::Value << "None";
				}
				else {
					std::string& normalPath = txl.getTextureByIndex(material.NormalTexture).path;
					out << YAML::Key << "NormalTexture" << YAML::Value << normalPath;
				}
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}

		if (entity->HasComponent<CameraComponent>()) {
			auto& cc = entity->GetComponent<CameraComponent>();

			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "MainCamera" << YAML::Value << cc.MainCamera;
			out << YAML::Key << "Projection" << YAML::Value << (int)cc.Projection;
			out << YAML::Key << "OrthographicLeft" << YAML::Value << cc.OrthographicLeft;
			out << YAML::Key << "OrthographicRight" << YAML::Value << cc.OrthographicRight;
			out << YAML::Key << "OrthographicBottom" << YAML::Value << cc.OrthographicBottom;
			out << YAML::Key << "OrthographicTop" << YAML::Value << cc.OrthographicTop;
			out << YAML::Key << "OrthographicNear" << YAML::Value << cc.OrthographicNear;
			out << YAML::Key << "OrthographicFar" << YAML::Value << cc.OrthographicFar;
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << cc.PerspectiveFOV;
			out << YAML::Key << "PerspectiveNear" << YAML::Value << cc.PerspectiveNear;
			out << YAML::Key << "PerspectiveFar" << YAML::Value << cc.PerspectiveFar;
			out << YAML::EndMap;
		}

		// Camera component needs a lot of work in general before it's ready for serialization
		/*if (entity->HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "CameraType" << YAML::Value << entity->GetComponent<CameraComponent>().Projection;
			out << YAML::EndMap;
		}*/

		out << YAML::EndMap;
	}


	// Conversions for YAML serialization
}

