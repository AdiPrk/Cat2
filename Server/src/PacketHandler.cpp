#include <PCH/pch.h>
#include "PacketHandler.h"
#include "PlayerManager.h"
#include "Server.h"

namespace Dog
{
    PacketHandler::PacketHandler(PacketUtils& utils) : packetUtils(utils) {}
    PacketHandler::~PacketHandler() {}

    void PacketHandler::HandlePacket(ENetPeer* peer, ENetPacket* packet, ENetHost* host, PlayerManager& playerManager) {
        int packetID;
        char data[PACKET_BUFFER_SIZE] = {};

        // Parse the packet data: an integer packet ID and optional string data.
        // (Note: For production code, consider a more robust parsing/serialization system.)
        sscanf((char*)packet->data, "%d %[^\t\n]", &packetID, data);

        switch (packetID) {
        case INIT_PLAYER_PACKET: {
            std::string playerName = playerManager.getPlayerName(peer);
            printf("Init Player %s\n", playerName.c_str());
            packetUtils.broadcastPacket(host, peer, INIT_PLAYER_PACKET, playerName.c_str());
            packetUtils.sendPacket(peer, SELF_NAME_PACKET, playerName.c_str());
            break;
        }
        case STARTED_TYPING_PACKET: {
            std::string user = playerManager.getPlayerName(peer);
            packetUtils.broadcastPacket(host, peer, STARTED_TYPING_PACKET, user.c_str());
            break;
        }
        case STOPPED_TYPING_PACKET: {
            std::string user = playerManager.getPlayerName(peer);
            packetUtils.broadcastPacket(host, peer, STOPPED_TYPING_PACKET, user.c_str());
            break;
        }
        case CHAT_MESSAGE_PACKET: {
            std::string message(data);
            std::string user = playerManager.getPlayerName(peer);
            std::string fullMessage = user + ": " + message;
            packetUtils.broadcastPacket(host, peer, CHAT_MESSAGE_PACKET, fullMessage.c_str());
            break;
        }
        case CHAT_DISPLAY_NAME_PACKET: {
            std::string name(data);
            std::string oldName = playerManager.getPlayerName(peer);
            playerManager.updatePlayerName(peer, name);
            std::string nameChange = oldName + "," + name;
            packetUtils.broadcastPacket(host, peer, CHAT_DISPLAY_NAME_PACKET, nameChange.c_str());
            break;
        }
        case CLIENT_LEAVE_PACKET: {
            printf("Client %s left.\n", data);
            std::string user = playerManager.getPlayerName(peer);
            packetUtils.broadcastPacket(host, peer, REMOVE_PLAYER_PACKET, user.c_str());
            playerManager.popPlayer(peer);
            break;
        }
        case EVENT_ACTION: {
            packetUtils.broadcastPacket(host, peer, EVENT_ACTION, data);           
            break;
        }
        case EVENT_ACTION_UNDO: {
            packetUtils.broadcastPacket(host, peer, EVENT_ACTION_UNDO, data);
            break;
        }
        default: {
            printf("Unknown packet ID: %d\n", packetID);
            break;
        }
        }
    }
}
