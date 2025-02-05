#pragma once

#include <enet/enet.h>
#include "PacketHandler.h"
#include "PlayerManager.h"

namespace Dog
{
    class Networking
    {
    public:
        Networking(const std::string& address, uint16_t port);
        ~Networking();

        void Init();
        void Shutdown();

        const std::string& GetUsername() const;
        bool IsConnected() const { return m_ConnectionStatus == ConnectionStatus::CONNECTED; }
        ConnectionStatus GetStatus() { return m_ConnectionStatus; }
        const std::vector<Player>& GetOtherClients() const { return playerManager.GetOtherClients(); }

    private:
        void InitializeENet();
        ENetPeer* CreateAndConnectClient();
        
        void networkThread();

        ENetPeer* m_Peer;

        std::thread m_NetworkThread;
        bool running = true;

        ConnectionStatus m_ConnectionStatus = ConnectionStatus::DISCONNECTED;

        PacketHandler packetHandler;
        PacketUtils m_PacketUtils;
        PlayerManager playerManager;

        std::string m_Address;
        uint16_t m_Port;
    };


}