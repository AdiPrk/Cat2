#include <PCH/pch.h>
#include "ActionManager.h"
#include "Engine.h"
//#include "Core/Scene/SceneManager.h"
//#include "Core/Scene/Scene.h"
#include "ECS/Entities/Components.h"
#include "ECS/Entities/Entity.h"
#include "Actions.h"

namespace Dog
{
    ActionManager::ActionManager(size_t maxSize)
        : m_MaxSize(maxSize)
        , m_CurrentIndex(0)
    {
        SubscribeAction<Event::EntityMoved, MoveEntityAction>();
        SubscribeAction<Event::EntityRotated, RotateEntityAction>();
        SubscribeAction<Event::EntityScaled, ScaleEntityAction>();
        SubscribeAction<Event::EntityTransform, TransformEntityAction>();
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

        // Add the new action
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
}
