#include <PCH/pch.h>
#include "SerializationResource.h"

#include "ECS/ECS.h"
#include "ECS/Entities/Entity.h"
#include "ECS/Entities/Components.h"

#include "rfl.hpp"
#include "Utils/SerializationOperators.h"

namespace Dog
{
    // Iterates over a type_list and applies a function for each type w/ fold expressions
    template <typename... Types, typename Func>
    void for_each_type(entt::type_list<Types...>, Func&& func) {
        (func.template operator()<Types> (), ...);
    }

    void SerializationResource::Serialize(const std::string& filepath)
    {
        // List of components to serialize
        entt::type_list<TagComponent, TransformComponent, ModelComponent, CameraComponent> types;

        // Get all entities
        entt::registry& registry = ecs->GetRegistry();
        auto view = registry.view<entt::entity>();

        nlohmann::json entitiesJson;

        for (auto& entityHandle : view)
        {
            nlohmann::json entityJson;

            for_each_type(types, [&]<typename ComponentType>() 
            {
                if (auto* component = registry.try_get<ComponentType>(entityHandle))
                {
                    auto cName = entt::type_name<ComponentType>::value();
                    if (const auto pos = cName.rfind("::"); pos != std::string_view::npos) {
                        cName = cName.substr(pos + 2);
                    }

                    nlohmann::json componentJson;
                    auto component_view = rfl::to_view(*component);
                    component_view.apply([&](const auto& f) 
                    {
                        componentJson[f.name()] = *f.value();
                    });

                    entityJson[cName].push_back(componentJson);
                }
            });

            entitiesJson["Entities"].push_back(entityJson);
        }

        std::ofstream file(filepath);
        if (file.is_open())
        {
            file << entitiesJson.dump(2);
            file.close();
            DOG_INFO("Scene serialized to {0}", filepath);
        }
        else
        {
            DOG_ERROR("Failed to open file {0} for writing", filepath);
        }
    }

    void SerializationResource::Deserialize(const std::string& filepath)
    {
    }
}
