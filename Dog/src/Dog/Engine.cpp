#include <PCH/pch.h>
#include "Engine.h"

#include "ECS/Systems/InputSystem.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/EditorSystem.h"
#include "ECS/Systems/PresentSystem.h"
#include "ECS/Systems/CameraSystem.h"

#include "ECS/Resources/InputResource.h"
#include "ECS/Resources/WindowResource.h"
#include "ECS/Resources/RenderingResource.h"
#include "ECS/Resources/EditorResource.h"
#include "ECS/Resources/SerializationResource.h"

#include "Graphics/Vulkan/Core/Device.h"
#include "Graphics/Window/FrameRate.h"
#include "Graphics/Vulkan/Pipeline/Pipeline.h"

namespace Dog 
{
    Engine::Engine(const EngineSpec& specs)
        : mSpecs(specs)
        , mEcs()
    {
        Logger::Init();

        // Systems -------------------------
        mEcs.AddSystem<InputSystem>();
        mEcs.AddSystem<CameraSystem>();
        mEcs.AddSystem<PresentSystem>();
        mEcs.AddSystem<RenderSystem>();
        mEcs.AddSystem<EditorSystem>();
        // ---------------------------------

        // Resources -----------------------
        mEcs.CreateResource<WindowResource>(specs.width, specs.height, specs.name);

        auto wr = mEcs.GetResource<WindowResource>();
        mEcs.CreateResource<InputResource>(wr->window->getGLFWwindow());
        mEcs.CreateResource<RenderingResource>(*wr->window);

        auto rr = mEcs.GetResource<RenderingResource>();
        mEcs.CreateResource<EditorResource>(*rr->device, *rr->swapChain, *wr->window);
        mEcs.CreateResource<SerializationResource>();
        // ---------------------------------

        // Initialize ECS
        mEcs.Init();
    }

    Engine::~Engine() 
    {
    }

    int Engine::Run(const std::string& sceneName) 
    {
        FrameRateController frameRateController(mSpecs.fps);

        while (!mEcs.GetResource<WindowResource>()->window->shouldClose() && mRunning) 
        {
            float dt = frameRateController.WaitForNextFrame();

            mEcs.FrameStart();
            mEcs.Update(dt);
            mEcs.FrameEnd();
        }

        vkDeviceWaitIdle(mEcs.GetResource<RenderingResource>()->device->getDevice());
        mEcs.Exit();

        return EXIT_SUCCESS;
    }

    void Engine::Exit()
    {
        mRunning = false;
    }

} // namespace Dog