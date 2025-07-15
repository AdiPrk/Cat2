#pragma once

#include "ISystem.h"

namespace Dog
{
    class Entity;

    class ECS
    {
    public:
        ECS();
        ~ECS();

        ECS(const ECS&) = delete;
        ECS& operator=(const ECS&) = delete;
        ECS(ECS&&) = delete;
        ECS& operator=(ECS&&) = delete;

        void FrameStart();
        void Update(float dt);
        void FrameEnd();
        
        // Add System
        template<typename T>
        void AddSystem()
        {
            static_assert(std::is_base_of<ISystem, T>::value, "T must inherit from ISystem");
            systems.emplace_back(std::make_unique<T>());
        }

    private:
        std::vector<std::unique_ptr<ISystem>> systems;

    };
}
