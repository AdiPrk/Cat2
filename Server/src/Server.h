#pragma once

#include "PlayerManager.h"
#include "PacketHandler.h"

namespace Dog
{

    class Server {
    public:
        // Creates a server listening on the given port.
        Server(unsigned short port);
        ~Server();

        // Runs the server’s main loop.
        void run();


    private:
        ENetHost* host;
        PlayerManager playerManager;
        PacketUtils m_PacketUtils;
        PacketHandler packetHandler;

        // Helper functions for processing ENet events.
        void handleConnect(ENetEvent& event);
        void handleReceive(ENetEvent& event);
        void handleDisconnect(ENetEvent& event);
        
    };

}
