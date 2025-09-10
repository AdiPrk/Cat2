#include <PCH/pch.h>
#include "RenderSystem.h"
#include "../ECS.h"

#include "../Resources/RenderingResource.h"
#include "Graphics/Vulkan/Renderer.h"

namespace Dog
{
    void RenderSystem::Init()
    {
    }
    
    void RenderSystem::FrameStart()
    {
    }
    
    void RenderSystem::Update(float)
    {
        ecs->GetResource<RenderingResource>()->renderer->drawFrame();
    }
    
    void RenderSystem::FrameEnd()
    {
    }

    void RenderSystem::Exit()
    {
    }
}
