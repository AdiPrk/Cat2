#include <PCH/pch.h>
#include "ECS.h"

#include "Resources/IResource.h"
#include "Entities/Entity.h"
#include "Entities/Components.h"

//#include "Scene/SceneManager.h"

namespace Dog
{
    ECS::ECS()
        : systems()
        , mEntityMap()
        , mRegistry()
    {
    }

    ECS::~ECS()
    {
    }

    void ECS::Init()
    {
        // Run Init
        for (auto& resource : mResources)
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

    void ECS::AddEntity(const std::string& name)
    {
        Entity entity(&mRegistry);

        entity.AddComponent<TagComponent>(name);
        entity.AddComponent<TransformComponent>();

        mEntityMap[name] = entity;
    }

    Entity ECS::GetEntity(const std::string& name)
    {
        auto it = mEntityMap.find(name);
        if (it == mEntityMap.end())
        {
            DOG_WARN("Entity with name '{0}' not found!", name);
            return nullptr;
        }

        return it->second;
    }

}
