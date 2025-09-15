#include <PCH/pch.h>
#include "Engine.h"

#include "Input/KeyboardController.h"
//#include "Graphics/Vulkan/Buffers/Buffer.h"
//#include "Graphics/Vulkan/Systems/IndirectRenderer.h"
//#include "Graphics/Vulkan/Texture/Texture.h"
//#include "Graphics/Vulkan/Texture/ImGuiTexture.h"
#include "Graphics/Vulkan/Pipeline/Pipeline.h"

#include "glslang/Public/ShaderLang.h"

#include "Graphics/Window/FrameRate.h"

#include "ECS/Entities/Entity.h"
#include "ECS/Entities/Components.h"

#include "Graphics/OldEditor/Editor.h"
#include "Assets/FileWatcher/FileWatcher.h"

#include "Networking/Networking.h"
#include "Events/Actions.h"

#include "ECS/Systems/TestSystem.h"
#include "ECS/Systems/InputSystem.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/EditorSystem.h"
#include "ECS/Systems/PresentSystem.h"

#include "ECS/Resources/InputResource.h"
#include "ECS/Resources/WindowResource.h"
#include "ECS/Resources/RenderingResource.h"
#include "ECS/Resources/EditorResource.h"

#include "Graphics/Vulkan/Core/Device.h"

namespace Dog {

    Engine::Engine(const EngineSpec& specs)
        : fps(specs.fps)
        //, m_Window(specs.width, specs.height, specs.name)
        //, m_Renderer(std::make_unique<Renderer>(m_Window, device))
        //, textureLibrary(device)
        //, modelLibrary(device, textureLibrary)
        , ecs()
    {
        Logger::Init();

        ecs.AddSystem<TestSystem>();
        ecs.AddSystem<InputSystem>();
        ecs.AddSystem<PresentSystem>();
        ecs.AddSystem<RenderSystem>();
        ecs.AddSystem<EditorSystem>();

        ecs.CreateResource<WindowResource>(specs.width, specs.height, specs.name);
        ecs.CreateResource<InputResource>(ecs.GetResource<WindowResource>()->window->getGLFWwindow());
        ecs.CreateResource<RenderingResource>(*ecs.GetResource<WindowResource>()->window);
        ecs.CreateResource<EditorResource>();

        ecs.Init();

        //m_ActionManager = std::make_unique<ActionManager>();
        //m_Editor = std::make_unique<Editor>(device);
        //m_Networking = std::make_unique<Networking>(specs.serverAddress, specs.serverPort);
    }

    Engine::~Engine() 
    {
        //m_Editor->Exit();
    }

    int Engine::Run(const std::string& sceneName) 
    {
        // Init some stuff
        //m_Networking->Init();

        WATCH_DIRECTORY(Texture);

        FrameRateController frameRateController(fps);

        while (!ecs.GetResource<WindowResource>()->window->shouldClose() && m_Running) 
        {
            float dt = frameRateController.WaitForNextFrame();

            ecs.FrameStart();
            ecs.Update(dt);
            ecs.FrameEnd();
        }

        vkDeviceWaitIdle(ecs.GetResource<RenderingResource>()->device->getDevice());

        //m_Networking->Shutdown();
        ecs.Exit();
        
        return EXIT_SUCCESS;
    }

    void Engine::Exit()
    {
        m_Running = false;
    }

} // namespace Dog