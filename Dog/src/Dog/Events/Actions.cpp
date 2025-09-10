#include <PCH/pch.h>
#include "Actions.h"
#include "Engine.h"
#include "Core/Scene/SceneManager.h"
#include "Core/Scene/Scene.h"
#include "Core/Scene/Entity/Components.h"
#include "Core/Scene/Entity/Entity.h"
#include "Networking/Networking.h"

namespace Dog
{

    MoveEntityAction::MoveEntityAction(Event::EntityMoved e, bool propogate)
        : entityID(e.entityID), oldX(e.oldX), oldY(e.oldY), oldZ(e.oldZ), newX(e.newX), newY(e.newY), newZ(e.newZ)
    {
        if (propogate) {
            std::string packet = Serialize();
            Engine::Get().GetNetworking().SendAction(packet);
        }
    }

    void MoveEntityAction::Apply()
    {
        //auto& transform = SceneManager::GetCurrentScene()->GetRegistry().get<TransformComponent>((entt::entity)entityID);
        //transform.Translation = { newX, newY, newZ };
    }
    
    void MoveEntityAction::Undo()
    {
        //auto& transform = SceneManager::GetCurrentScene()->GetRegistry().get<TransformComponent>((entt::entity)entityID);
        //transform.Translation = { oldX, oldY, oldZ };
    }
    
    std::string MoveEntityAction::Serialize() const
    {
        std::stringstream ss;
        ss << EVENT_ENTITY_MOVE_PACKET << " " << entityID << " " << oldX << " " << oldY << " " << oldZ << " " << newX << " " << newY << " " << newZ;
        return ss.str();
    }

    RotateEntityAction::RotateEntityAction(Event::EntityRotated e, bool propogate)
        : entityID(e.entityID), oldX(e.oldX), oldY(e.oldY), oldZ(e.oldZ), newX(e.newX), newY(e.newY), newZ(e.newZ)
    {
        if (propogate) {
            std::string packet = Serialize();
            Engine::Get().GetNetworking().SendAction(packet);
        }
    }

    void RotateEntityAction::Apply()
    {
        //auto& transform = SceneManager::GetCurrentScene()->GetRegistry().get<TransformComponent>((entt::entity)entityID);
        //transform.Rotation = { newX, newY, newZ };
    }

    void RotateEntityAction::Undo()
    {
        //auto& transform = SceneManager::GetCurrentScene()->GetRegistry().get<TransformComponent>((entt::entity)entityID);
        //transform.Rotation = { oldX, oldY, oldZ };
    }

    std::string RotateEntityAction::Serialize() const
    {
        std::stringstream ss;
        ss << EVENT_ENTITY_ROTATE_PACKET << " " << entityID << " " << oldX << " " << oldY << " " << oldZ << " " << newX << " " << newY << " " << newZ;
        return ss.str();
    }

    ScaleEntityAction::ScaleEntityAction(Event::EntityScaled e, bool propogate)
        : entityID(e.entityID), oldX(e.oldX), oldY(e.oldY), oldZ(e.oldZ), newX(e.newX), newY(e.newY), newZ(e.newZ)
    {
        if (propogate) {
            std::string packet = Serialize();
            Engine::Get().GetNetworking().SendAction(packet);
        }
    }

    void ScaleEntityAction::Apply()
    {
        //auto& transform = SceneManager::GetCurrentScene()->GetRegistry().get<TransformComponent>((entt::entity)entityID);
        //transform.Scale = { newX, newY, newZ };
    }

    void ScaleEntityAction::Undo()
    {
        //auto& transform = SceneManager::GetCurrentScene()->GetRegistry().get<TransformComponent>((entt::entity)entityID);
        //transform.Scale = { oldX, oldY, oldZ };
    }

    std::string ScaleEntityAction::Serialize() const
    {
        std::stringstream ss;
        ss << EVENT_ENTITY_SCALE_PACKET << " " << entityID << " " << oldX << " " << oldY << " " << oldZ << " " << newX << " " << newY << " " << newZ;
        return ss.str();
    }

    TransformEntityAction::TransformEntityAction(Event::EntityTransform e, bool propogate)
        : entityID(e.entityID), oPX(e.oPX), oPY(e.oPY), oPZ(e.oPZ), oRX(e.oRX), oRY(e.oRY), oRZ(e.oRZ), oSX(e.oSX), oSY(e.oSY), oSZ(e.oSZ), nPX(e.nPX), nPY(e.nPY), nPZ(e.nPZ), nRX(e.nRX), nRY(e.nRY), nRZ(e.nRZ), nSX(e.nSX), nSY(e.nSY), nSZ(e.nSZ)
    {
        if (propogate) {
            std::string packet = Serialize();
            Engine::Get().GetNetworking().SendAction(packet);
        }
    }

    void TransformEntityAction::Apply()
    {
        //auto& transform = SceneManager::GetCurrentScene()->GetRegistry().get<TransformComponent>((entt::entity)entityID);
        //transform.Translation = { nPX, nPY, nPZ };
        //transform.Rotation = { nRX, nRY, nRZ };
        //transform.Scale = { nSX, nSY, nSZ };
    }

    void TransformEntityAction::Undo()
    {
        //auto& transform = SceneManager::GetCurrentScene()->GetRegistry().get<TransformComponent>((entt::entity)entityID);
        //transform.Translation = { oPX, oPY, oPZ };
        //transform.Rotation = { oRX, oRY, oRZ };
        //transform.Scale = { oSX, oSY, oSZ };
    }

    std::string TransformEntityAction::Serialize() const
    {
        std::stringstream ss;
        ss << EVENT_ENTITY_TRANSFORM_PACKET << " " << entityID << " " << oPX << " " << oPY << " " << oPZ << " " << oRX << " " << oRY << " " << oRZ << " " << oSX << " " << oSY << " " << oSZ << " " << nPX << " " << nPY << " " << nPZ << " " << nRX << " " << nRY << " " << nRZ << " " << nSX << " " << nSY << " " << nSZ;
        return ss.str();
    }
}
