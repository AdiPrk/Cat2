#include <PCH/pch.h>
#include "scene.h"
#include "Dog/Logger/logger.h"

#include "Entity/entity.h"
#include "Entity/components.h"
#include "Dog/engine.h"

#include "Dog/Graphics/Vulkan/Window/Window.h"

#define DOG_SCENE_LOGGING 0

namespace Dog {

	Scene::Scene(const std::string& name)
	{
		sceneName = name;

		/*FrameBufferSpecification fbSpec;
		fbSpec.width = 1280;
		fbSpec.height = 720;
		fbSpec.samples = 1;
		fbSpec.attachments = { FBAttachment::RGBA8, FBAttachment::Depth24Stencil8 };
		sceneFrameBuffer = std::make_shared<FrameBuffer>(fbSpec);*/

		width = Engine::Get().GetWindow().GetWidth();
		height = Engine::Get().GetWindow().GetHeight();

		// Shader::SetResolutionUBO(glm::vec2(fbSpec.width, fbSpec.height));

		//sceneOrthographicCamera = std::make_shared<SceneOrthographicCamera>((float)width / (float)height);
		//sceneOrthographicCamera->UpdateUniforms(); // Should be moved to the renderer (?) or somewhere else

		//scenePerspectiveCamera = std::make_shared<ScenePerspectiveCamera>();
		//scenePerspectiveCamera->UpdateUniforms(); // Should be moved to the renderer (?) or somewhere else

		// eventSceneFBResize = SUBSCRIBE_EVENT(Event::SceneResize, sceneFrameBuffer->OnSceneResize);
		// eventSceneOrthoCamResize = SUBSCRIBE_EVENT(Event::SceneResize, sceneOrthographicCamera->OnSceneResize);
		// eventScenePerspCamResize = SUBSCRIBE_EVENT(Event::SceneResize, scenePerspectiveCamera->OnSceneResize);
		// eventPlayButtonPressed = SUBSCRIBE_EVENT(Event::PlayButtonPressed, OnPlayButtonPressed);
		// eventStopButtonPressed = SUBSCRIBE_EVENT(Event::StopButtonPressed, OnStopButtonPressed);

#ifndef DOG_SHIP
		// Engine::GetEditor()->DoSceneResize();
#endif
	}

	Scene::~Scene()
	{
	}

	void Scene::OnPlayButtonPressed(const Event::PlayButtonPressed& event)
	{
		DOG_INFO("Play button pressed.");
	}

	void Scene::OnStopButtonPressed(const Event::StopButtonPressed& event)
	{
		DOG_INFO("Stop button pressed.");
	}

	Entity Scene::GetPrimaryCamera()
	{
		auto view = registry.view<CameraComponent>();

		for (auto entity : view) {
			if (view.get<CameraComponent>(entity).MainCamera) {
				return Entity{ this, entity };
			}
		}

		return {};
	}

	void Scene::ClearEntities()
	{
		// reset registry
		registry.clear();
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity newEnt(this);

		newEnt.AddComponent<UUID>();
		TagComponent& tg = newEnt.AddComponent<TagComponent>(name);
		TransformComponent& tr = newEnt.AddComponent<TransformComponent>();
		ModelComponent& mc = newEnt.AddComponent<ModelComponent>("assets/models/quad.obj");

		return newEnt;
	}

	Entity Scene::CreateEntityFromUUID(const UUID& uuid, const std::string& name)
	{
		Entity newEnt(this);

		newEnt.AddComponent<UUID>(uuid);
		TagComponent& tg = newEnt.AddComponent<TagComponent>(name);
		TransformComponent& tr = newEnt.AddComponent<TransformComponent>();

		return newEnt;
	}

	Entity Scene::CreateEmptyEntity(const std::string& name)
	{
		Entity newEnt(this);

		newEnt.AddComponent<UUID>();
		TagComponent& tg = newEnt.AddComponent<TagComponent>(name);

		return newEnt;
	}

	void Scene::InternalInit()
	{
#if DOG_SCENE_LOGGING
		DOG_INFO("Scene {0} init.", sceneName);
#endif
	}

	void Scene::InternalUpdate(float dt)
	{
#if DOG_SCENE_LOGGING
		DOG_INFO("Scene {0} Update.", sceneName);
#endif

		/*if (isOrthographic) {
			sceneOrthographicCamera->OnUpdate(dt);
			sceneOrthographicCamera->UpdateUniforms();
		}
		else {
			scenePerspectiveCamera->OnUpdate(dt);
			scenePerspectiveCamera->UpdateUniforms();
		}*/
	}

	void Scene::InternalRender(float dt, bool renderEditor)
	{
#if DOG_SCENE_LOGGING
		DOG_INFO("Scene {0} Render.", sceneName);
#endif

		Engine::Get().GetRenderer().Render(dt);
	}

	void Scene::InternalExit()
	{
#if DOG_SCENE_LOGGING
		DOG_INFO("Scene {0} exit.", sceneName);
#endif
	}

	glm::mat4 Scene::GetProjectionMatrix()
	{
		/*if (isOrthographic) {
			return sceneOrthographicCamera->GetProjectionMatrix();
		}
		else {
			return scenePerspectiveCamera->GetProjectionMatrix();
		}*/
		return glm::mat4(1.f);
	}

	glm::mat4 Scene::GetViewMatrix()
	{
		/*if (isOrthographic) {
			return sceneOrthographicCamera->GetViewMatrix();
		}
		else {
			return scenePerspectiveCamera->GetViewMatrix();
		}*/
		return glm::mat4(1.f);
	}

}
