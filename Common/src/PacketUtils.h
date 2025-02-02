#pragma once
#include "enet/enet.h"
#include "Common.h"

namespace Dog
{
    class PacketUtils {
    public:
        PacketUtils();
        ~PacketUtils();

        // --- Sending functions ---
        void sendPacket(ENetPeer* peer, PacketID packetID);
        void sendPacket(ENetPeer* peer, PacketID packetID, const char* data);
        void sendPacketInt(ENetPeer* peer, PacketID packetID, int data);
        void sendPacketVec2(ENetPeer* peer, PacketID packetID, float x, float y);
        void sendPacketVec3(ENetPeer* peer, PacketID packetID, float x, float y, float z);

        // --- Broadcasting functions ---
        void broadcastPacket(ENetHost* host, ENetPeer* sender, PacketID packetID);
        void broadcastPacket(ENetHost* host, ENetPeer* sender, PacketID packetID, const char* data);
        void broadcastPacketInt(ENetHost* host, ENetPeer* sender, PacketID packetID, int data);
        void broadcastPacketVec2(ENetHost* host, ENetPeer* sender, PacketID packetID, float x, float y);
        void broadcastPacketVec3(ENetHost* host, ENetPeer* sender, PacketID packetID, float x, float y, float z);
    };
}
