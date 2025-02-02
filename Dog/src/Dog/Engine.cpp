#include <PCH/pch.h>
#include "Engine.h"

#include "Input/KeyboardController.h"
#include "Graphics/Vulkan/Buffers/Buffer.h"
#include "Graphics/Vulkan/Systems/IndirectRenderer.h"
#include "Graphics/Vulkan/Texture/Texture.h"
#include "Graphics/Vulkan/Texture/ImGuiTexture.h"

#include "glslang/Public/ShaderLang.h"

#include "Input/input.h"

#include "Graphics/Vulkan/Window/FrameRate.h"

#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Entity.h"
#include "Scene/Entity/Components.h"

#include "Graphics/Editor/Editor.h"
#include "Assets/FileWatcher/FileWatcher.h"

#include "Networking/Networking.h"

namespace Dog {

    Engine::Engine(const EngineSpec& specs)
        : m_Window(specs.width, specs.height, specs.name)
        , m_Renderer(std::make_unique<Renderer>(m_Window, device))
        , textureLibrary(device)
        , modelLibrary(device, textureLibrary)
        , fps(specs.fps)
    {
        Logger::Init();
        m_Editor = std::make_unique<Editor>();
        m_Networking = std::make_unique<Networking>(specs.serverAddress, specs.serverPort);
    }

    Engine::~Engine() 
    {
        m_Editor->Exit();
    }

    int Engine::Run(const std::string& sceneName) 
    {
        // Init some stuff
        m_Networking->Init();
        m_Editor->Init();
        m_Renderer->Init();

        SceneManager::SetNextScene(sceneName);
        SceneManager::SwapScenes(); // Init the first scene

        WATCH_DIRECTORY(Texture);

        FrameRateController frameRateController(fps);

        auto currentTime = std::chrono::high_resolution_clock::now();
        while (!m_Window.shouldClose() && m_Running) {
            Input::Update();
            float dt = frameRateController.WaitForNextFrame();
            
            // Need to move in frame rate controller
            // auto newTime = std::chrono::high_resolution_clock::now();
            // float dt = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            // currentTime = newTime;

            // Swap scenes if necessary (also does Init/Exit)
            SceneManager::SwapScenes();

            // Update scenes
            SceneManager::Update(dt);

            // Render scenes
            SceneManager::Render(dt, true);
        }

        m_Renderer->Exit();

        m_Networking->Shutdown();
        
        return EXIT_SUCCESS;
    }

    void Engine::Exit()
    {
        m_Running = false;
    }

} // namespace Dog