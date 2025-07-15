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

#include "Core/Scene/SceneManager.h"
#include "Core/Scene/Scene.h"
#include "Core/Scene/Entity/Entity.h"
#include "Core/Scene/Entity/Components.h"

#include "Graphics/Editor/Editor.h"
#include "Assets/FileWatcher/FileWatcher.h"

#include "Networking/Networking.h"
#include "Events/Actions.h"

namespace Dog {

    Engine::Engine(const EngineSpec& specs)
        : m_Window(specs.width, specs.height, specs.name)
        , m_Renderer(std::make_unique<Renderer>(m_Window, device))
        , textureLibrary(device)
        , modelLibrary(device, textureLibrary)
        , fps(specs.fps)
        , ecs()
    {
        Logger::Init();
        m_ActionManager = std::make_unique<ActionManager>();
        m_Editor = std::make_unique<Editor>(device);
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

        while (!m_Window.shouldClose() && m_Running) 
        {
            Input::Update();
            float dt = frameRateController.WaitForNextFrame();

            ecs.FrameStart();
            ecs.Update(dt);
            ecs.FrameEnd();
        }

        vkDeviceWaitIdle(device);

        m_Renderer->Exit();
        m_Networking->Shutdown();
        
        return EXIT_SUCCESS;
    }

    void Engine::Exit()
    {
        m_Running = false;
    }

} // namespace Dog