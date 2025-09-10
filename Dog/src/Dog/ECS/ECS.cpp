#include <PCH/pch.h>
#include "ECS.h"

#include "Resources/IResource.h"

//#include "Scene/SceneManager.h"

namespace Dog
{
    ECS::ECS()
        : systems()
    {
    }

    ECS::~ECS()
    {
    }

    void ECS::Init()
    {
        // Run Init
        for (auto& resource : m_Resources)
        {
            resource.second->ecs = this;
        }

        for (auto& system : systems)
        {
            system->ecs = this;
            system->Init();
        }
    }

    void ECS::FrameStart()
    {
        for (auto& system : systems)
        {
            system->FrameStart();
        }
    }

    void ECS::Update(float dt)
    {
        for (auto& system : systems)
        {
            system->Update(dt);
        }
    }

    void ECS::FrameEnd()
    {
        for (auto& system : systems)
        {
            system->FrameEnd();
        }
    }

    void ECS::Exit()
    {
        for (auto& system : systems)
        {
            system->Exit();
        }
    }
}
