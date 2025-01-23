#pragma once

#include "SceneManager.h"

namespace Dog {

	class FrameBuffer;
	class OrthographicCamera;
	class SceneOrthographicCamera;
	class ScenePerspectiveCamera;
	class SceneSerializer;
	class Entity;

	class Scene {
	public:
		Scene(const std::string&);
		~Scene();

		virtual void Init() {}
		virtual void Update(float dt) {}
		virtual void Render(float dt) {}
		virtual void Exit() {}

		const std::string& GetName() const { return sceneName; }

		void ClearEntities();
		Entity CreateEntity(const std::string& name = "Unnamed Entity");
		Entity CreateEntityFromUUID(const UUID& uuid, const std::string& name = "Unnamed Entity");
		Entity CreateEmptyEntity(const std::string& name = "Unnamed Entity");

		const std::shared_ptr<FrameBuffer>& GetFrameBuffer() const { return sceneFrameBuffer; }
		std::shared_ptr<FrameBuffer>& GetFrameBuffer() { return sceneFrameBuffer; }

		void OnPlayButtonPressed(const Event::PlayButtonPressed& event);
		void OnStopButtonPressed(const Event::StopButtonPressed& event);

		Entity GetPrimaryCamera();

		// Entity related stuff
		entt::registry& GetRegistry() { return registry; }
		entt::registry registry;

		std::unordered_map<std::string, Entity> entityMap;
	private:
		// Scene name only used for debug purposes
		std::string sceneName;

		// Scene width & height, equal to window width & height
		unsigned int width;
		unsigned int height;

		// Framebuffer for the editor to render the scene
		std::shared_ptr<FrameBuffer> sceneFrameBuffer;

		// Functions that run before or after the user's scene functions
		friend SceneManager;
		void InternalInit();
		void InternalUpdate(float dt);
		void InternalRender(float dt, bool renderEditor);
		void InternalExit();

		// Serializer
		friend SceneSerializer;

		// Scene camera
		friend class Input;
		std::shared_ptr<SceneOrthographicCamera> sceneOrthographicCamera;
		std::shared_ptr<ScenePerspectiveCamera> scenePerspectiveCamera;
		glm::mat4 GetProjectionMatrix();
		glm::mat4 GetViewMatrix();
		bool isOrthographic = false;

		// Event handles
		Events::Handle<Event::SceneResize> eventSceneFBResize;
		Events::Handle<Event::SceneResize> eventSceneOrthoCamResize;
		Events::Handle<Event::SceneResize> eventScenePerspCamResize;
		Events::Handle<Event::PlayButtonPressed> eventPlayButtonPressed;
		Events::Handle<Event::StopButtonPressed> eventStopButtonPressed;
	};

}
