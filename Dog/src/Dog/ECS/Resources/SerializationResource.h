#pragma once

#include "IResource.h"

namespace Dog
{
    struct TagComponent;
    struct TransformComponent;
    struct ModelComponent;
    struct CameraComponent;
    struct AnimationComponent;

    struct SerializationResource : public IResource
    {
        entt::type_list<TagComponent, TransformComponent, ModelComponent, CameraComponent, AnimationComponent> types;

        void Serialize(const std::string& filepath);
        void Deserialize(const std::string& filepath);
    };
}
