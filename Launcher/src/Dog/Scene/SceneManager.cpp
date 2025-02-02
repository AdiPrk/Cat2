#include <PCH/pch.h>
#include "SceneManager.h"
#include "Scene.h"
#include "Serializer/SceneSerializer.h"
//#include "Dog/Assets/Packer/assetPacker.h"

namespace Dog {

	SceneManager SceneManager::m_Instance;
	Scene* SceneManager::m_ActiveScene = nullptr;
	std::string SceneManager::m_NextScene;
	bool SceneManager::m_IsRestarting = false;

	void SceneManager::SetNextScene(const std::string& next)
	{
		if (m_NextScene == next) {
			m_IsRestarting = true;
		}
		else {
			m_NextScene = next;
		}
	}

	void SceneManager::Init()
	{
	}

	void SceneManager::SwapScenes()
	{
		// Check for a scene change.
		if (!m_NextScene.empty())
		{
			// Exit the current scene.
			Exit();

			// delete old scene
			if (m_ActiveScene) {
				delete m_ActiveScene;
			}

			// Create the new scene.
			m_ActiveScene = new Scene(m_NextScene);

			// load it
			SceneSerializer::Deserialize(m_ActiveScene, "assets/scenes/" + m_NextScene + ".yaml");

			m_NextScene.clear();

			// Initialize the new scene.
			m_ActiveScene->InternalInit();
			m_ActiveScene->Init();
		}
	}

	void SceneManager::Update(float dt)
	{
		if (!m_ActiveScene) return;

		// Update the active scene.
		m_ActiveScene->InternalUpdate(dt);
		m_ActiveScene->Update(dt);
	}

	void SceneManager::Render(float dt, bool renderEditor)
	{
		if (!m_ActiveScene) return;

		m_ActiveScene->InternalRender(dt, renderEditor);
		m_ActiveScene->Render(dt);
	}

	void SceneManager::Exit(void)
	{
		if (m_ActiveScene) {
			m_ActiveScene->Exit();
			m_ActiveScene->InternalExit();
		}
	}

	bool SceneManager::IsChangingScenes()
	{
		if (!m_ActiveScene) return true;
		return m_IsRestarting || (m_ActiveScene->GetName() != m_NextScene);
	}

}