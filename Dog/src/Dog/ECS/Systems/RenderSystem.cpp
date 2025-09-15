#include <PCH/pch.h>
#include "RenderSystem.h"

#include "../Resources/RenderingResource.h"
#include "Graphics/Vulkan/Renderer.h"
#include "Graphics/Vulkan/Model/ModelLibrary.h"

#include "../ECS.h"
#include "ECS/Entities/Entity.h"
#include "ECS/Entities/Components.h"

namespace Dog
{
    void RenderSystem::Init()
    {
        ecs->AddEntity("Hii");

        Entity ent = ecs->GetEntity("Hii");
        if (ent)
        {
            ModelComponent& mc = ent.AddComponent<ModelComponent>();
            mc.ModelIndex = 0;
        }

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
