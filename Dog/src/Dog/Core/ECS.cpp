#include <PCH/pch.h>
#include "ECS.h"

#include "Systems/TestSystem.h"

#include "Scene/SceneManager.h"

namespace Dog
{
    ECS::ECS()
        : systems()
    {
        // Register systems
        AddSystem<TestSystem>();

        // Run Init
        for (auto& system : systems)
        {
            system->Init();
        }
    }

    ECS::~ECS()
    {
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

        // TEMPORARILY;;
        SceneManager::Update(dt);
        SceneManager::Render(dt, true);
    }

    void ECS::FrameEnd()
    {
        for (auto& system : systems)
        {
            system->FrameEnd();
        }
    }
}
