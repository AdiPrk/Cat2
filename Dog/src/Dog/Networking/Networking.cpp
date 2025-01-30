#include <PCH/pch.h>
#include "Networking.h"
#include "Graphics/Editor/Editor.h"
#include "Graphics/Editor/Windows/ChatWindow.h"
#include "Engine.h"

namespace Dog {

    // Function definitions
    void Networking::initializeENet()
    {
        if (enet_initialize() != 0)
        {
            fprintf(stderr, "An error occurred while initializing ENet!\n");
            return;
        }
        atexit(enet_deinitialize);
    }

    ENetPeer* Networking::createAndConnectClient()
    {
        initializeENet();

        ENetHost* client;
        client = enet_host_create(NULL, 1, 1, 0, 0);

        if (client == NULL)
        {
            fprintf(stderr, "An error occurred while trying to create an ENet client host!\n");
            return nullptr;
        }

        ENetAddress address;
        ENetEvent event;

        enet_address_set_host(&address, "localhost");
        address.port = 7777;

        m_Peer = enet_host_connect(client, &address, 1, 0);
        if (m_Peer == NULL)
        {
            fprintf(stderr, "No available peers for initiating an ENet connection!\n");
            return nullptr;
        }

        // Wait up to 5 seconds for the connection attempt to succeed
        printf("Trying to connect to server...");
        if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
            printf("Connection to server succeeded.\n");
        }
        else {
            enet_peer_reset(m_Peer);
            fprintf(stderr, "Connection to server failed.\n");
            m_Peer = nullptr;
            return nullptr;
        }

        return m_Peer;
    }

    void Networking::Init()
    {
        if (!createAndConnectClient()) {
            m_Connected = false;
            return;
        }

        sendPacket(m_Peer, INIT_PLAYER_PACKET);

        m_NetworkThread = std::thread(&Networking::networkThread, this, m_Peer);
        m_Connected = true;
    }

    void Networking::Shutdown()
    {
        enet_peer_disconnect(m_Peer, 0);
        running = false;

        if (m_NetworkThread.joinable())
        {
            m_NetworkThread.join();
        }
    }

    void Networking::sendPacket(ENetPeer* peer, PacketID packetID)
    {
        char packetData[256];
        snprintf(packetData, sizeof(packetData), "%d", packetID);
        ENetPacket* packet = enet_packet_create(packetData, strlen(packetData) + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
    }

    void Networking::sendPacket(ENetPeer* peer, PacketID packetID, const char* data) {
        char packetData[256];
        snprintf(packetData, sizeof(packetData), "%d %s", packetID, data);
        ENetPacket* packet = enet_packet_create(packetData, strlen(packetData) + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
    }

    void Networking::StartTyping()
    {
        sendPacket(m_Peer, STARTED_TYPING_PACKET);
    }

    void Networking::StopTyping()
    {
        sendPacket(m_Peer, STOPPED_TYPING_PACKET);
    }

    void Networking::SendMessage(const std::string& message)
    {
        sendPacket(m_Peer, CHAT_MESSAGE_PACKET, message.c_str());
    }

    void Networking::SetUsername(const std::string& name)
    {
        sendPacket(m_Peer, CHAT_DISPLAY_NAME_PACKET, name.c_str());
        m_Username = name;
    }

    void Networking::ChangeClientName(const std::string& oldName, const std::string& newName)
    {
        // remove if it's in there and replace with new
        auto it = m_OtherClients.find(oldName);
        if (it != m_OtherClients.end())
        {
            m_OtherClients.erase(it);
            m_OtherClients[newName] = newName;
        }
    }

    std::string Networking::GetUsername()
    {
        return m_Username;
    }

    void Networking::handlePacket(ENetPeer* peer, ENetPacket* packet)
    {
        int packetID;
        char data[256] = {};

        // Reading the packetID as an integer and the rest of the packet as string data
        int numArgs = sscanf_s((char*)packet->data, "%d %[^\t\n]", &packetID, data, (unsigned)_countof(data));

        if (numArgs < 2) { // Check if both arguments were successfully read
            fprintf(stderr, "Error: Could not parse the packet correctly.\n");
            return;
        }

        printf("Packet id: %d\n", packetID);

        switch (packetID) {
        case INIT_PLAYER_PACKET: 
        {
            AddClient(data);

            break;
        }
        case SELF_NAME_PACKET:
        {
            m_Username = data;
            break;
        }
        case REMOVE_PLAYER_PACKET: {
            
            RemoveClient(data);

            break;
        }
        case CHAT_MESSAGE_PACKET:
        {
            printf("Received message: %s\n", data);
            ChatWindow* chatWindow = Engine::Get().GetEditor().GetChatWindow();
            if (chatWindow)
            {
                std::string message(data);
                std::string sender = message.substr(0, message.find(":"));
                std::string messageText = message.substr(message.find(":") + 2);

                chatWindow->AddMessage(sender, messageText);
            }

            break;
        }
        case STARTED_TYPING_PACKET:
        {
            ChatWindow* chatWindow = Engine::Get().GetEditor().GetChatWindow();
            if (chatWindow)
            {
                chatWindow->UserStartedTyping(data);
            }

            break;
        }
        case STOPPED_TYPING_PACKET:
        {
            ChatWindow* chatWindow = Engine::Get().GetEditor().GetChatWindow();
            if (chatWindow)
            {
                chatWindow->UserStoppedTyping(data);
            }

            break;
        }
        case CHAT_DISPLAY_NAME_PACKET: 
        {
            ChatWindow* chatWindow = Engine::Get().GetEditor().GetChatWindow();
            if (chatWindow)
            {
                std::string nameChange(data);
                std::string oldName = nameChange.substr(0, nameChange.find(","));
                std::string newName = nameChange.substr(nameChange.find(",") + 1);
                chatWindow->AddMessage("Server", oldName + " changed their name to " + newName);
                chatWindow->UserChangedName(oldName);
                ChangeClientName(oldName, newName);
            }
            break;
        }
        }
    }

    void Networking::networkThread(ENetPeer* peer) {
        ENetEvent event;
        while (running) {
            int eventStatus = enet_host_service(peer->host, &event, 10); // Wait up to 16 milliseconds for an event

            if (eventStatus > 0) {
                switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE:
                    // Handle received packet
                    handlePacket(peer, event.packet);
                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    // Handle disconnection
                    printf("Disconnected from server.\n");
                    enet_host_destroy(peer->host);
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

    void Networking::AddClient(std::string name)
    {
        m_OtherClients[name] = name;
        printf("Init player %s\n", name.c_str());
    }

    void Networking::RemoveClient(std::string id)
    {
        auto it = m_OtherClients.find(id);
        if (it != m_OtherClients.end())
        {
            m_OtherClients.erase(it);

            printf("Remove player %s\n", id.c_str());
        }
    }

} // namespace Dog
