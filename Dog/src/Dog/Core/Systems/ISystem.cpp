#include <PCH/pch.h>
#include "ISystem.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"

namespace Dog
{
    entt::registry& ISystem::GetECS()
    {
        if (SceneManager::GetCurrentScene() == nullptr)
        {
            throw std::runtime_error("No active scene found. Please ensure a scene is loaded before accessing the ECS.");
        }

        return SceneManager::GetCurrentScene()->GetRegistry();
    }
}
