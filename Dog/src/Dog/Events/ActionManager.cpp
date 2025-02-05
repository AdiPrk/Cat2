#include <PCH/pch.h>
#include "ActionManager.h"
#include "Engine.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components.h"
#include "Scene/Entity/Entity.h"
#include "Actions.h"

namespace Dog
{
    ActionManager::ActionManager(size_t maxSize)
        : m_MaxSize(maxSize)
        , m_CurrentIndex(0)
    {
        m_EntityMovedHandle = SUBSCRIBE_EVENT(Event::EntityMoved, OnEntityMoved);
        m_EntityRotatedHandle = SUBSCRIBE_EVENT(Event::EntityRotated, OnEntityRotated);
        m_EntityScaledHandle = SUBSCRIBE_EVENT(Event::EntityScaled, OnEntityScaled);
        m_EntityTransformHandle = SUBSCRIBE_EVENT(Event::EntityTransform, OnEntityTransform);
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

    const Action* ActionManager::GetCurrentAction()
    {
        if (m_Actions.empty() || m_CurrentIndex >= m_Actions.size())
        {
            return nullptr;
        }

        return m_Actions[m_CurrentIndex].get();
    }

    const Action* ActionManager::GetLastAction()
    {
        if (m_Actions.empty() || m_CurrentIndex <= 0)
        {
            return nullptr;
        }

        return m_Actions[m_CurrentIndex - 1].get();
    }

    void ActionManager::OnEntityMoved(const Event::EntityMoved& event)
    {
        AddAction(std::make_unique<MoveEntityAction>(event.entityID, event.oldX, event.oldY, event.oldZ, event.newX, event.newY, event.newZ));
    }

    void ActionManager::OnEntityRotated(const Event::EntityRotated& event)
    {
        AddAction(std::make_unique<RotateEntityAction>(event.entityID, event.oldX, event.oldY, event.oldZ, event.newX, event.newY, event.newZ));
    }

    void ActionManager::OnEntityScaled(const Event::EntityScaled& event)
    {
        AddAction(std::make_unique<ScaleEntityAction>(event.entityID, event.oldX, event.oldY, event.oldZ, event.newX, event.newY, event.newZ));
    }
    void ActionManager::OnEntityTransform(const Event::EntityTransform& event)
    {
        AddAction(std::make_unique<TransformEntityAction>(event.entityID, event.oPX, event.oPY, event.oPZ, event.oRX, event.oRY, event.oRZ, event.oSX, event.oSY, event.oSZ, event.nPX, event.nPY, event.nPZ, event.nRX, event.nRY, event.nRZ, event.nSX, event.nSY, event.nSZ));
    }
}
