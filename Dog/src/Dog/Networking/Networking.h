#pragma once

#include <enet/enet.h>

namespace Dog
{
    // Define packet IDs
    enum PacketID {
        INIT_PLAYER_PACKET = 1,
        SELF_NAME_PACKET = 2,
        REMOVE_PLAYER_PACKET = 3,
        STARTED_TYPING_PACKET = 4,
        STOPPED_TYPING_PACKET = 5,
        CHAT_MESSAGE_PACKET = 6,
        CHAT_DISPLAY_NAME_PACKET = 7,
    };

    class Networking
    {
    public:
        void Init();
        void Shutdown();

        void sendPacket(ENetPeer* peer, PacketID packetID);
        void sendPacket(ENetPeer* peer, PacketID packetID, const char* data);

        void StartTyping();
        void StopTyping();
        void SendMessage(const std::string& message);
        void SetUsername(const std::string& name);
        void ChangeClientName(const std::string& oldName, const std::string& newName);

        std::string GetUsername();
        bool IsConnected() { return m_Connected; }
        const std::unordered_map<std::string, std::string>& GetOtherClients() const { return m_OtherClients; }

    private:
        void initializeENet();
        ENetPeer* createAndConnectClient();

        void handlePacket(ENetPeer* peer, ENetPacket* packet);
        
        void networkThread(ENetPeer* peer);

        ENetPeer* m_Peer;
        std::thread m_NetworkThread;

        std::unordered_map<std::string, std::string> m_OtherClients;
        void AddClient(std::string id);
        void RemoveClient(std::string id);

        bool running = true;
        std::string m_Username;

        bool m_Connected = false;
    };


}