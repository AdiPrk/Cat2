#pragma once

//#include "Graphics/Vulkan/Descriptors/Descriptors.h"
//#include "Graphics/Vulkan/Core/Device.h"
//#include "Graphics/Vulkan/Renderer.h"
//#include "Graphics/Vulkan/Texture/TextureLibrary.h"
//#include "Graphics/Vulkan/Models/ModelLibrary.h"
//#include "Graphics/Vulkan/Animation/Animation.h"
//#include "Graphics/Vulkan/Animation/Animator.h"

#include "Graphics/Window/Window.h"

#include "ECS/ECS.h"

#include "ECS/Resources/IResource.h"

namespace Dog {
	// Forward Declarations
	class Networking;
	class ActionManager;

	struct EngineSpec {
		std::string name = "Dog Engine";         // The name of the window.
		unsigned width = 1280;                   // The width of the window.
		unsigned height = 720;                   // The height of the window.
		unsigned fps = 60;			             // The target frames per second.
        std::string serverAddress = SERVER_IP;   // The address of the server. Defaults to online VPS server.
        uint16_t serverPort = 7777;              // The port of the server.
	};

	class Editor;

	class Engine {
	public:
		static constexpr int WIDTH = 1600;
		static constexpr int HEIGHT = 900;

		Engine(const EngineSpec& specs);
		~Engine();

		// Called by client to create the engine.
		static Engine& Create(const EngineSpec& specs = {})
		{
			static Engine instance(specs);
			return instance;
		}

		static Engine& Get()
		{
			return Create();
		}


		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

		/*********************************************************************
		 * param:  sceneName: The name of the scene to run. (read from assets/scenes)
		 * 
		 * brief: Run the engine with the specified scene.
		 *********************************************************************/
		int Run(const std::string& sceneName);
		void Exit();

		// getters
        Networking& GetNetworking() { return *m_Networking; }
        ActionManager& GetActionManager() { return *m_ActionManager; }

	private:
		// Networking
        std::unique_ptr<Networking> m_Networking;

		// Actions
        std::unique_ptr<ActionManager> m_ActionManager;

		// target fps
		unsigned fps;

		// running
		bool m_Running = true;

		// Entity Component System for the engine
        ECS ecs; 
	};

} // namespace Dog
