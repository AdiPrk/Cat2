#include <PCH/pch.h>
#include "Actions.h"
#include "Engine.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components.h"

namespace Dog
{
    ActionManager::ActionManager(size_t maxSize) 
        : m_MaxSize(maxSize)
        , m_CurrentIndex(0) 
    {
        m_EntityMovedHandle = SUBSCRIBE_EVENT(Event::EntityMoved, OnEntityMoved);
    }
    
    void ActionManager::AddAction(std::unique_ptr<Action> action)
    {
        // If we're adding a new action, remove all actions ahead of the m_CurrentIndex
        if (m_CurrentIndex < m_Actions.size())
        {
            m_Actions.erase(m_Actions.begin() + m_CurrentIndex, m_Actions.end());
        }

        if (m_Actions.size() >= m_MaxSize)
        {
            m_Actions.erase(m_Actions.begin());
            m_CurrentIndex = std::max(m_CurrentIndex - 1, 0u);
        }

        // Add the new action and apply it
        m_Actions.push_back(std::move(action));
        ++m_CurrentIndex;
    }
    
    void ActionManager::Undo()
    {
        if (m_CurrentIndex > 0)
        {
            --m_CurrentIndex;
            m_Actions[m_CurrentIndex]->Undo();
        }
    }

    void ActionManager::Redo()
    {
        if (m_CurrentIndex < m_Actions.size())
        {
            m_Actions[m_CurrentIndex]->Apply();
            m_CurrentIndex++;
        }
    }
    
    void ActionManager::Clear()
    {
        m_Actions.clear();
        m_CurrentIndex = 0;
    }

    void ActionManager::OnEntityMoved(const Event::EntityMoved& event)
    {
        AddAction(std::make_unique<MoveEntityAction>(event.entityID, event.oldX, event.oldY, event.oldZ, event.newX, event.newY, event.newZ));
    }

    MoveEntityAction::MoveEntityAction(uint32_t entityID, float oldX, float oldY, float oldZ, float newX, float newY, float newZ)
        : entityID(entityID), oldX(oldX), oldY(oldY), oldZ(oldZ), newX(newX), newY(newY), newZ(newZ)
    {
    }

    void MoveEntityAction::Apply()
    {
        auto scene = SceneManager::GetCurrentScene();
        auto& transform = scene->GetRegistry().get<TransformComponent>((entt::entity)m_ID);
        transform.Translation = { newX, newY, newZ };
    }
    
    void MoveEntityAction::Undo()
    {
    }
    
    std::string MoveEntityAction::Serialize() const
    {
        return std::string();
    }
}
