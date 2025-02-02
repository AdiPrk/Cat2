#include <PCH/pch.h>
#include "PacketHandler.h"
#include "PlayerManager.h"
#include "Graphics/Editor/Editor.h"
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
            playerManager.RemoveClient(data);
            break;
        }
        case CHAT_MESSAGE_PACKET:
        case STARTED_TYPING_PACKET:
        case STOPPED_TYPING_PACKET:
        case CHAT_DISPLAY_NAME_PACKET:
        {
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
