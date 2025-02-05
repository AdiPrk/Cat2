#pragma once

namespace Dog
{
    struct Player
    {
        std::string id;
        std::string name;
    };

    class PlayerManager
    {
    public:
        PlayerManager();
        ~PlayerManager();

        void AddClient(std::string id);
        void RemoveClient(std::string id);
        void SetUsername(std::string name) { m_Client.name = name; }
        void ChangeClientName(const std::string& oldName, const std::string& newName);

        const std::string& GetUsername() const { return m_Client.name; }
        const std::vector<Player>& GetOtherClients() const { return m_OtherClients; }

    private:
        std::vector<Player> m_OtherClients;
        Player m_Client;
    };
}
