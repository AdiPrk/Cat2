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
        , ecs()
    {
        Logger::Init();

        // Systems -------------------------
        ecs.AddSystem<TestSystem>();
        ecs.AddSystem<InputSystem>();
        ecs.AddSystem<PresentSystem>();
        ecs.AddSystem<RenderSystem>();
        ecs.AddSystem<EditorSystem>();
        // ---------------------------------

        // Resources -----------------------
        ecs.CreateResource<WindowResource>(specs.width, specs.height, specs.name);

        auto wr = ecs.GetResource<WindowResource>();
        ecs.CreateResource<InputResource>(wr->window->getGLFWwindow());
        ecs.CreateResource<RenderingResource>(*wr->window);

        auto rr = ecs.GetResource<RenderingResource>();
        ecs.CreateResource<EditorResource>(*rr->device, *rr->swapChain, *wr->window);
        // ---------------------------------

        // Initialize ECS
        ecs.Init();
    }

    Engine::~Engine() 
    {
    }

    int Engine::Run(const std::string& sceneName) 
    {
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
        ecs.Exit();        

        return EXIT_SUCCESS;
    }

    void Engine::Exit()
    {
        m_Running = false;
    }

} // namespace Dog