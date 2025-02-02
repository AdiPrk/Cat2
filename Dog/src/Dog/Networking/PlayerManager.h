#pragma once

namespace Dog
{
    class PlayerManager
    {
    public:
        PlayerManager();
        ~PlayerManager();

        void AddClient(std::string id);
        void RemoveClient(std::string id);
        void SetUsername(std::string name) { m_Username = name; }
        void ChangeClientName(const std::string& oldName, const std::string& newName);

        const std::string& GetUsername() const { return m_Username; }
        const std::unordered_map<std::string, std::string>& GetOtherClients() const { return m_OtherClients; }

    private:
        std::unordered_map<std::string, std::string> m_OtherClients;
        std::string m_Username;
    };
}
