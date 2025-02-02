#include <PCH/pch.h>
#include "PacketHandler.h"
#include "PlayerManager.h"
#include "Graphics/Editor/Editor.h"
#include "Graphics/Editor/Windows/ChatWindow.h"
#include "Engine.h"

namespace Dog
{
    PacketHandler::PacketHandler(PacketUtils& utils) : packetUtils(utils) {}
    PacketHandler::~PacketHandler() {}

    void PacketHandler::HandlePacket(ENetPeer* peer, ENetPacket* packet, PlayerManager& playerManager) {
        int packetID;
        char data[PACKET_BUFFER_SIZE] = {};

        // Parse the packet data: an integer packet ID and optional string data.
        // (Note: For production code, consider a more robust parsing/serialization system.)
        sscanf((char*)packet->data, "%d %[^\t\n]", &packetID, data);

        printf("Packet id: %d\n", packetID);

        switch (packetID) {
        case INIT_PLAYER_PACKET:
        {
            playerManager.AddClient(data);
            break;
        }
        case SELF_NAME_PACKET:
        {
            playerManager.SetUsername(data);
            break;
        }
        case REMOVE_PLAYER_PACKET: {
            ChatWindow* chatWindow = Engine::Get().GetEditor().GetChatWindow();
            if (chatWindow)
            {
                chatWindow->AddMessage("Server", std::string(data) + " has left the chat.");
                chatWindow->UserStoppedTyping(data);
            }
            playerManager.RemoveClient(data);
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
                playerManager.ChangeClientName(oldName, newName);
            }
            break;
        }
        case CLIENT_LEAVE_PACKET:
        {
            // we shouldn't ever get this anyway
            break;
        }
        default:
        {
            printf("Unknown packet ID: %d\n", packetID);
            break;
        }
        }
    }

}
