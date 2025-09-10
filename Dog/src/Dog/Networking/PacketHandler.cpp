#include <PCH/pch.h>
#include "PacketHandler.h"
#include "PlayerManager.h"
#include "Graphics/Editor/Editor.h"
#include "Graphics/Editor/Windows/ChatWindow.h"
#include "Events/Actions.h"
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
            //ChatWindow* chatWindow = Engine::Get().GetEditor().GetChatWindow();
            //if (chatWindow)
            //{
            //    chatWindow->AddMessage("Server", std::string(data) + " has left the chat.");
            //    chatWindow->UserStoppedTyping(data);
            //}
            playerManager.RemoveClient(data);
            break;
        }
        case CHAT_MESSAGE_PACKET:
        {
            printf("Received message: %s\n", data);
            //ChatWindow* chatWindow = Engine::Get().GetEditor().GetChatWindow();
            //if (chatWindow)
            //{
            //    std::string message(data);
            //    std::string sender = message.substr(0, message.find(":"));
            //    std::string messageText = message.substr(message.find(":") + 2);
            //
            //    chatWindow->AddMessage(sender, messageText);
            //}

            break;
        }
        case STARTED_TYPING_PACKET:
        {
            //ChatWindow* chatWindow = Engine::Get().GetEditor().GetChatWindow();
            //if (chatWindow)
            //{
            //    chatWindow->UserStartedTyping(data);
            //}

            break;
        }
        case STOPPED_TYPING_PACKET:
        {
            //ChatWindow* chatWindow = Engine::Get().GetEditor().GetChatWindow();
            //if (chatWindow)
            //{
            //    chatWindow->UserStoppedTyping(data);
            //}

            break;
        }
        case CHAT_DISPLAY_NAME_PACKET:
        {
            //ChatWindow* chatWindow = Engine::Get().GetEditor().GetChatWindow();
            //if (chatWindow)
            //{
            //    std::string nameChange(data);
            //    std::string oldName = nameChange.substr(0, nameChange.find(","));
            //    std::string newName = nameChange.substr(nameChange.find(",") + 1);
            //    chatWindow->AddMessage("Server", oldName + " changed their name to " + newName);
            //    chatWindow->UserChangedName(oldName);
            //    playerManager.ChangeClientName(oldName, newName);
            //}
            break;
        }
        case CLIENT_LEAVE_PACKET:
        {
            // we shouldn't ever get this anyway
            break;
        }
        case EVENT_ACTION:
        {
            std::string action(data);
            HandleEventPacket(action, peer, packet, playerManager);
            break;
        }
        case EVENT_ACTION_UNDO:
        {
            std::string action(data);
            HandleEventPacket(action, peer, packet, playerManager, true);
            break;
        }
        default:
        {
            printf("Unknown packet ID: %d\n", packetID);
            break;
        }
        }
    }

    void PacketHandler::HandleEventPacket(const std::string& event, ENetPeer* peer, ENetPacket* packet, PlayerManager& playerManager, bool undo)
    {
        // Create an input string stream from the action string.
        std::istringstream iss(event);

        int eventType;
        if (!(iss >> eventType)) {
            // malformed packet
            return;
        }

        switch (eventType)
        {
        case EVENT_ENTITY_MOVE_PACKET:
        {
            uint32_t entityID;
            float oldX, oldY, oldZ, newX, newY, newZ;
            if (!(iss >> entityID >> oldX >> oldY >> oldZ >> newX >> newY >> newZ)) return;

            Event::EntityMoved e = { entityID, oldX, oldY, oldZ, newX, newY, newZ };
            MoveEntityAction action(e, false);
            if (undo) action.Undo();
            else action.Apply();

            break;
        }
        case EVENT_ENTITY_ROTATE_PACKET:
        {
            uint32_t entityID;
            float oldX, oldY, oldZ, newX, newY, newZ;
            if (!(iss >> entityID >> oldX >> oldY >> oldZ >> newX >> newY >> newZ)) return;
            
            Event::EntityRotated e = { entityID, oldX, oldY, oldZ, newX, newY, newZ };
            RotateEntityAction action(e, false);
            if (undo) action.Undo();
            else action.Apply();

            break;
        }
        case EVENT_ENTITY_SCALE_PACKET:
        {
            uint32_t entityID;
            float oldX, oldY, oldZ, newX, newY, newZ;
            if (!(iss >> entityID >> oldX >> oldY >> oldZ >> newX >> newY >> newZ)) return;
            
            Event::EntityScaled e = { entityID, oldX, oldY, oldZ, newX, newY, newZ };
            ScaleEntityAction action(e, false);
            if (undo) action.Undo();
            else action.Apply();

            break;
        }
        case EVENT_ENTITY_TRANSFORM_PACKET:
        {
            uint32_t entityID;
            float oPX, oPY, oPZ, oRX, oRY, oRZ, oSX, oSY, oSZ, nPX, nPY, nPZ, nRX, nRY, nRZ, nSX, nSY, nSZ;
            if (!(iss >> entityID >> oPX >> oPY >> oPZ >> oRX >> oRY >> oRZ >> oSX >> oSY >> oSZ >> nPX >> nPY >> nPZ >> nRX >> nRY >> nRZ >> nSX >> nSY >> nSZ)) return;
            
            Event::EntityTransform e = { entityID, oPX, oPY, oPZ, oRX, oRY, oRZ, oSX, oSY, oSZ, nPX, nPY, nPZ, nRX, nRY, nRZ, nSX, nSY, nSZ };
            TransformEntityAction action(e, false);
            if (undo) action.Undo();
            else action.Apply();

            break;
        }
        default:
            DOG_WARN("Unknown event action type: {0}", eventType);
            break;
        }
    }

}
