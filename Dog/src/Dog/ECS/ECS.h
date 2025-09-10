#pragma once

#include "Systems/ISystem.h"

namespace Dog
{
    class Entity;
    struct IResource;

    class ECS
    {
    public:
        ECS();
        ~ECS();

        ECS(const ECS&) = delete;
        ECS& operator=(const ECS&) = delete;
        ECS(ECS&&) = delete;
        ECS& operator=(ECS&&) = delete;

        void Init();
        void FrameStart();
        void Update(float dt);
        void FrameEnd();
        void Exit();
        
        // Add System
        template<typename T>
        void AddSystem()
        {
            static_assert(std::is_base_of<ISystem, T>::value, "T must inherit from ISystem");
            systems.emplace_back(std::make_unique<T>());
        }

        template<typename T, typename... Args>
        void CreateResource(Args&&... args) {
            static_assert(std::is_base_of<IResource, T>::value, "Resource type must inherit from Dog::IResource");

            const auto typeId = std::type_index(typeid(T));

            auto [it, inserted] = m_Resources.try_emplace(typeId, std::make_unique<T>(std::forward<Args>(args)...));

            if (!inserted) {
                DOG_ERROR("Resource of type '{0}' already exists.", typeid(T).name());
            }
        }

        // Gets a pointer to the resource of the specified type.
        template<typename T>
        T* GetResource() {
            const auto typeId = std::type_index(typeid(T));
            auto it = m_Resources.find(typeId);

            if (it == m_Resources.end()) return nullptr;

            return dynamic_cast<T*>(it->second.get());
        }

    private:
        std::vector<std::unique_ptr<ISystem>> systems;
        std::unordered_map<std::type_index, std::unique_ptr<IResource>> m_Resources;
    };
}
