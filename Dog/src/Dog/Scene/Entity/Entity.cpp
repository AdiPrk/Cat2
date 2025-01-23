#include <PCH/pch.h>

#include "entity.h"

#include "../scene.h"



namespace Dog {
    Entity::Entity()
        : scene(nullptr)
        , handle(entt::null)
    {
    }

    Entity::Entity(Scene* scene_)
    {
        scene = scene_;
        handle = scene->GetRegistry().create();
    }

    Entity::Entity(Scene* scene, entt::entity handle)
        : scene(scene)
        , handle(handle)
    {
    }

    Entity::Entity(const Entity& other)
    {
        scene = other.scene;
        handle = other.handle;
    }

    void Entity::operator=(const Entity& other)
    {
        scene = other.scene;
        handle = other.handle;
    }

    Entity::~Entity() {
        // Shouldn't get destroyed here.
        // The scene should destroy it in the scene's destructor
        // Or when asked by the user

        // scene->GetRegistry().destroy(handle);
    }

}