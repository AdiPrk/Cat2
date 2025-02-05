#include <PCH/pch.h>
#include "PlayerManager.h"

namespace Dog
{
    PlayerManager::PlayerManager() {}
    PlayerManager::~PlayerManager() {}

    void PlayerManager::AddClient(std::string name)
    {
        Player client;
        client.id = name;
        client.name = name;
        m_OtherClients.push_back(client);

        printf("Init player %s\n", name.c_str());
    }

    void PlayerManager::RemoveClient(std::string id)
    {
        // Erase-remove idiom: remove all players whose id matches the provided id.
        m_OtherClients.erase(
            std::remove_if(m_OtherClients.begin(), m_OtherClients.end(),
                [&id](const Player& p) { return p.id == id; }),
            m_OtherClients.end()
        );
    }

    void PlayerManager::ChangeClientName(const std::string& oldName, const std::string& newName)
    {
        for (auto& client : m_OtherClients)
        {
            if (client.name == oldName)
            {
                client.name = newName;
                break;
            }
        }
    }
}
