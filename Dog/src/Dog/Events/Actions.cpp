#include <PCH/pch.h>
#include "Actions.h"
#include "Engine.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components.h"
#include "Scene/Entity/Entity.h"

namespace Dog
{

    MoveEntityAction::MoveEntityAction(uint32_t entityID, float oldX, float oldY, float oldZ, float newX, float newY, float newZ)
        : entityID(entityID), oldX(oldX), oldY(oldY), oldZ(oldZ), newX(newX), newY(newY), newZ(newZ)
    {
    }

    void MoveEntityAction::Apply()
    {
        auto& transform = SceneManager::GetCurrentScene()->GetRegistry().get<TransformComponent>((entt::entity)entityID);
        transform.Translation = { newX, newY, newZ };
    }
    
    void MoveEntityAction::Undo()
    {
        auto& transform = SceneManager::GetCurrentScene()->GetRegistry().get<TransformComponent>((entt::entity)entityID);
        transform.Translation = { oldX, oldY, oldZ };
    }
    
    std::string MoveEntityAction::Serialize() const
    {
        std::stringstream ss;
        ss << "EVENT_ENTITY_MOVE_PACKET" << " " << entityID << " " << oldX << " " << oldY << " " << oldZ << " " << newX << " " << newY << " " << newZ;
        return ss.str();
    }

    RotateEntityAction::RotateEntityAction(uint32_t entityID, float oldX, float oldY, float oldZ, float newX, float newY, float newZ)
        : entityID(entityID), oldX(oldX), oldY(oldY), oldZ(oldZ), newX(newX), newY(newY), newZ(newZ)
    {
    }

    void RotateEntityAction::Apply()
    {
        auto& transform = SceneManager::GetCurrentScene()->GetRegistry().get<TransformComponent>((entt::entity)entityID);
        transform.Rotation = { newX, newY, newZ };
    }

    void RotateEntityAction::Undo()
    {
        auto& transform = SceneManager::GetCurrentScene()->GetRegistry().get<TransformComponent>((entt::entity)entityID);
        transform.Rotation = { oldX, oldY, oldZ };
    }

    std::string RotateEntityAction::Serialize() const
    {
        std::stringstream ss;
        ss << "EVENT_ENTITY_ROTATE_PACKET" << " " << entityID << " " << oldX << " " << oldY << " " << oldZ << " " << newX << " " << newY << " " << newZ;
        return ss.str();
    }

    ScaleEntityAction::ScaleEntityAction(uint32_t entityID, float oldX, float oldY, float oldZ, float newX, float newY, float newZ)
        : entityID(entityID), oldX(oldX), oldY(oldY), oldZ(oldZ), newX(newX), newY(newY), newZ(newZ)
    {
    }

    void ScaleEntityAction::Apply()
    {
        auto& transform = SceneManager::GetCurrentScene()->GetRegistry().get<TransformComponent>((entt::entity)entityID);
        transform.Scale = { newX, newY, newZ };
    }

    void ScaleEntityAction::Undo()
    {
        auto& transform = SceneManager::GetCurrentScene()->GetRegistry().get<TransformComponent>((entt::entity)entityID);
        transform.Scale = { oldX, oldY, oldZ };
    }

    std::string ScaleEntityAction::Serialize() const
    {
        std::stringstream ss;
        ss << "EVENT_ENTITY_SCALE_PACKET" << " " << entityID << " " << oldX << " " << oldY << " " << oldZ << " " << newX << " " << newY << " " << newZ;
        return ss.str();
    }

    TransformEntityAction::TransformEntityAction(uint32_t entityID, float oPX, float oPY, float oPZ, float oRX, float oRY, float oRZ, float oSX, float oSY, float oSZ, float nPX, float nPY, float nPZ, float nRX, float nRY, float nRZ, float nSX, float nSY, float nSZ)
        : entityID(entityID), oPX(oPX), oPY(oPY), oPZ(oPZ), oRX(oRX), oRY(oRY), oRZ(oRZ), oSX(oSX), oSY(oSY), oSZ(oSZ), nPX(nPX), nPY(nPY), nPZ(nPZ), nRX(nRX), nRY(nRY), nRZ(nRZ), nSX(nSX), nSY(nSY), nSZ(nSZ)
    {
    }

    void TransformEntityAction::Apply()
    {
        auto& transform = SceneManager::GetCurrentScene()->GetRegistry().get<TransformComponent>((entt::entity)entityID);
        transform.Translation = { nPX, nPY, nPZ };
        transform.Rotation = { nRX, nRY, nRZ };
        transform.Scale = { nSX, nSY, nSZ };
    }

    void TransformEntityAction::Undo()
    {
        auto& transform = SceneManager::GetCurrentScene()->GetRegistry().get<TransformComponent>((entt::entity)entityID);
        transform.Translation = { oPX, oPY, oPZ };
        transform.Rotation = { oRX, oRY, oRZ };
        transform.Scale = { oSX, oSY, oSZ };
    }

    std::string TransformEntityAction::Serialize() const
    {
        std::stringstream ss;
        ss << "EVENT_ENTITY_TRANSFORM_PACKET" << " " << entityID << " " << oPX << " " << oPY << " " << oPZ << " " << oRX << " " << oRY << " " << oRZ << " " << oSX << " " << oSY << " " << oSZ << " " << nPX << " " << nPY << " " << nPZ << " " << nRX << " " << nRY << " " << nRZ << " " << nSX << " " << nSY << " " << nSZ;
        return ss.str();
    }
}
