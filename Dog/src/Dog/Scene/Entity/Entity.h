#pragma once

#include "../Scene.h"

namespace Dog {

    class Entity {
    public:
        Entity(); // an empty entity
        Entity(Scene* scene);
        Entity(Scene* scene, entt::entity handle);
        Entity(const Entity& other);
        void operator=(const Entity& other);
        ~Entity();

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args) {
            return scene->GetRegistry().emplace<T>(handle, std::forward<Args>(args)...);
        }

        template<typename T>
        T& GetComponent() {
            return scene->GetRegistry().get<T>(handle);
        }

        template<typename T>
        bool HasComponent() {
            return scene->GetRegistry().all_of<T>(handle);
        }

        bool operator==(const Entity& other) const {
            return handle == other.handle && scene == other.scene;
        }

        bool operator!=(const Entity& other) const {
            return !operator==(other);
        }

        operator bool() const {
            return handle != entt::null && scene != nullptr;
        }

        operator entt::entity() const {
            return handle;
        }

        operator int() const {
            return (int)handle;
        }

        Scene* GetScene() { return scene; }

    private:
        Scene* scene;
        entt::entity handle;
    };

    struct Parent {
        Entity parent;
    };

    struct Children {
        std::vector<Entity> children;
    };

}