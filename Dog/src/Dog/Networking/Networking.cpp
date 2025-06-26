#include <PCH/pch.h>
#include "Networking.h"
#include "Graphics/Editor/Editor.h"
#include "Graphics/Editor/Windows/ChatWindow.h"
#include "Engine.h"

namespace Dog {

    // Function definitions
    void Networking::InitializeENet()
    {
        if (enet_initialize() != 0)
        {
            fprintf(stderr, "An error occurred while initializing ENet!\n");
            return;
        }
        atexit(enet_deinitialize);
    }

    ENetPeer* Networking::CreateAndConnectClient()
    {
        InitializeENet();

        ENetHost* client;
        client = enet_host_create(NULL, 1, 1, 0, 0);

        if (client == NULL)
        {
            fprintf(stderr, "An error occurred while trying to create an ENet client host!\n");
            return nullptr;
        }

        ENetAddress address;
        ENetEvent event;

        enet_address_set_host(&address, m_Address.c_str());
        address.port = m_Port;

        m_Peer = enet_host_connect(client, &address, 1, 0);
        if (m_Peer == NULL)
        {
            fprintf(stderr, "No available peers for initiating an ENet connection!\n");
            return nullptr;
        }

        // Wait up to 10 seconds for the connection attempt to succeed
        printf("Trying to connect to server...");
        m_ConnectionStatus = ConnectionStatus::CONNECTING;
        if (enet_host_service(client, &event, 10000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
            printf("Connection to server succeeded.\n");
            m_ConnectionStatus = ConnectionStatus::CONNECTED;
        }
        else {
            enet_peer_reset(m_Peer);
            fprintf(stderr, "Connection to server failed.\n");
            m_Peer = nullptr;
            m_ConnectionStatus = ConnectionStatus::DISCONNECTED;
            return nullptr;
        }

        return m_Peer;
    }

    Networking::Networking(const std::string& address, uint16_t port)
        : m_PacketUtils()
        , packetHandler(m_PacketUtils)
        , m_Peer(nullptr)
        , m_Address(address)
        , m_Port(port)
    {
    }

    Networking::~Networking()
    {
    }

    void Networking::Init()
    {
        m_NetworkThread = std::thread(&Networking::networkThread, this);
    }

    void Networking::Shutdown()
    {
        if (m_Peer) {
            m_PacketUtils.sendPacket(m_Peer, CLIENT_LEAVE_PACKET);

            enet_host_flush(m_Peer->host);
            enet_peer_disconnect(m_Peer, 0);
            running = false;
        }

        if (m_NetworkThread.joinable())
        {
            m_NetworkThread.join();
        }
    }

    void Networking::StartTyping()
    {
        m_PacketUtils.sendPacket(m_Peer, STARTED_TYPING_PACKET);
    }

    void Networking::StopTyping()
    {
        m_PacketUtils.sendPacket(m_Peer, STOPPED_TYPING_PACKET);
    }

    void Networking::SendMessage(const std::string& message)
    {
        m_PacketUtils.sendPacket(m_Peer, CHAT_MESSAGE_PACKET, message.c_str());
    }

    void Networking::SetUsername(const std::string& name)
    {
        m_PacketUtils.sendPacket(m_Peer, CHAT_DISPLAY_NAME_PACKET, name.c_str());
        playerManager.SetUsername(name);
    }

    void Networking::SendAction(const std::string& action)
    {
        m_PacketUtils.sendPacket(m_Peer, EVENT_ACTION, action.c_str());
    }

    void Networking::SendUndoAction(const std::string& action)
    {
        m_PacketUtils.sendPacket(m_Peer, EVENT_ACTION_UNDO, action.c_str());
    }

    const std::string& Networking::GetUsername() const
    {
        return playerManager.GetUsername();
    }

    const std::unordered_map<std::string, std::string>& Networking::GetOtherClients() const
    {
        return playerManager.GetOtherClients();
    }

    void Networking::networkThread() {
        if (!CreateAndConnectClient()) {
            return;
        }

        m_PacketUtils.sendPacket(m_Peer, INIT_PLAYER_PACKET);

        ENetEvent event;
        while (running) {
            int eventStatus = enet_host_service(m_Peer->host, &event, 10); // Wait up to 16 milliseconds for an event

            if (eventStatus > 0) {
                switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE:
                    // Handle received packet
                    packetHandler.HandlePacket(m_Peer, event.packet, playerManager);
                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    // Handle disconnection
                    printf("Disconnected from server.\n");
                    enet_host_destroy(m_Peer->host);
                    enet_deinitialize();
                    return;
                }
            }
            else if (eventStatus == 0) {
                // No event, sleep for a short duration to reduce CPU usage
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

} // namespace Dog
