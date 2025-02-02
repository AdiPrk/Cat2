#include <PCH/pch.h>
#include "PlayerManager.h"

namespace Dog
{
    PlayerManager::PlayerManager() {}
    PlayerManager::~PlayerManager() {}

    void PlayerManager::AddClient(std::string name)
    {
        m_OtherClients[name] = name;
        printf("Init player %s\n", name.c_str());
    }

    void PlayerManager::RemoveClient(std::string id)
    {
        auto it = m_OtherClients.find(id);
        if (it != m_OtherClients.end())
        {
            m_OtherClients.erase(it);
            printf("Remove player %s\n", id.c_str());
        }
    }

    void PlayerManager::ChangeClientName(const std::string& oldName, const std::string& newName)
    {
        // remove if it's in there and replace with new
        auto it = m_OtherClients.find(oldName);
        if (it != m_OtherClients.end())
        {
            m_OtherClients.erase(it);
            m_OtherClients[newName] = newName;
        }
    }
}
