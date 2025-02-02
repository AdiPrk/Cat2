#pragma once

namespace Dog {

	class Scene;
	class Entity;
	class Renderer;

	class SceneManager
	{
	public:
		static SceneManager GetInstance() { return m_Instance; }
		static Scene* GetCurrentScene() { return m_ActiveScene; }
		static bool IsRunning() { return (m_ActiveScene || m_NextScene.empty()); }
		static void RestartCurrentScene() { m_IsRestarting = true; }
		static void SetNextScene(const std::string& next);

		static void Init();
		static void SwapScenes();
		static void Update(float dt);
		static void Render(float dt, bool renderEditor);
		static void Exit();

	private:
		static SceneManager m_Instance;
		static Scene* m_ActiveScene;
		static std::string m_NextScene;
		static bool m_IsRestarting;

		static bool IsChangingScenes();
	};
}