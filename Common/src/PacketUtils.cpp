#include "PacketUtils.h"
#include "Common.h"
#include <stdarg.h>
#include <cstdio>
#include <cstring>

namespace Dog
{
    // Helper: Create an ENetPacket using a formatted string.
    ENetPacket* createFormattedPacket(const char* format, va_list args) {
        char buffer[Dog::PACKET_BUFFER_SIZE];
        vsnprintf(buffer, PACKET_BUFFER_SIZE, format, args);
        size_t len = strlen(buffer) + 1;
        return enet_packet_create(buffer, len, ENET_PACKET_FLAG_RELIABLE);
    }

    // Overloaded helper that accepts variadic arguments.
    ENetPacket* createFormattedPacket(const char* format, ...) {
        va_list args;
        va_start(args, format);
        ENetPacket* packet = createFormattedPacket(format, args);
        va_end(args);
        return packet;
    }

    // Generic broadcast helper that takes a lambda which sends a packet to a peer.
    template<typename Fn>
    void broadcastHelper(ENetHost* host, ENetPeer* sender, Fn sendFunc) {
        for (size_t i = 0; i < host->peerCount; ++i) {
            ENetPeer* currentPeer = &host->peers[i];
            if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer == sender)
                continue;
            sendFunc(currentPeer);
        }
    }

    PacketUtils::PacketUtils() {}
    PacketUtils::~PacketUtils() {}

    // --- Sending functions ---

    void PacketUtils::sendPacket(ENetPeer* peer, PacketID packetID) {
        ENetPacket* packet = createFormattedPacket("%d", packetID);
        enet_peer_send(peer, 0, packet);
    }

    void PacketUtils::sendPacket(ENetPeer* peer, PacketID packetID, const char* data) {
        ENetPacket* packet = createFormattedPacket("%d %s", packetID, data);
        enet_peer_send(peer, 0, packet);
    }

    void PacketUtils::sendPacketInt(ENetPeer* peer, PacketID packetID, int data) {
        ENetPacket* packet = createFormattedPacket("%d %i", packetID, data);
        enet_peer_send(peer, 0, packet);
    }

    void PacketUtils::sendPacketVec2(ENetPeer* peer, PacketID packetID, float x, float y) {
        ENetPacket* packet = createFormattedPacket("%d %f %f", packetID, x, y);
        enet_peer_send(peer, 0, packet);
    }

    void PacketUtils::sendPacketVec3(ENetPeer* peer, PacketID packetID, float x, float y, float z) {
        ENetPacket* packet = createFormattedPacket("%d %f %f %f", packetID, x, y, z);
        enet_peer_send(peer, 0, packet);
    }

    // --- Broadcasting functions ---

    void PacketUtils::broadcastPacket(ENetHost* host, ENetPeer* sender, PacketID packetID) {
        broadcastHelper(host, sender, [this, packetID](ENetPeer* p) {
            sendPacket(p, packetID);
        });
    }

    void PacketUtils::broadcastPacket(ENetHost* host, ENetPeer* sender, PacketID packetID, const char* data) {
        broadcastHelper(host, sender, [this, packetID, data](ENetPeer* p) {
            sendPacket(p, packetID, data);
        });
    }

    void PacketUtils::broadcastPacketInt(ENetHost* host, ENetPeer* sender, PacketID packetID, int data) {
        broadcastHelper(host, sender, [this, packetID, data](ENetPeer* p) {
            sendPacketInt(p, packetID, data);
        });
    }

    void PacketUtils::broadcastPacketVec2(ENetHost* host, ENetPeer* sender, PacketID packetID, float x, float y) {
        broadcastHelper(host, sender, [this, packetID, x, y](ENetPeer* p) {
            sendPacketVec2(p, packetID, x, y);
        });
    }

    void PacketUtils::broadcastPacketVec3(ENetHost* host, ENetPeer* sender, PacketID packetID, float x, float y, float z) {
        broadcastHelper(host, sender, [this, packetID, x, y, z](ENetPeer* p) {
            sendPacketVec3(p, packetID, x, y, z);
        }); 
    }
}
