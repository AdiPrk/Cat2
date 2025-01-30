/*****************************************************************//**
 * \file   main.cpp
 * \author Adi (aditya.prakash@digipen.edu)
 * \date   January 29 2025
 * \Copyright @ 2024 Digipen (USA) Corporation * 
 
 * \brief  server code for engine
 *  *********************************************************************/

#include <stdio.h>
#include <enet/enet.h>
#include <vector>
#include <unordered_map>
#include <string>

float playerIDCounter = 0;

// Peer linked with an ID
std::unordered_map<ENetPeer*, std::string> Players;
int idCounter = 0;

void PushPlayer(ENetPeer* peer)
{
	Players[peer] = std::to_string(idCounter++);
}

void PopPlayer(ENetPeer* peer)
{
	auto it = Players.find(peer);
	if (it != Players.end()) {
		Players.erase(it);
	}
}

// Define packet IDs
enum PacketID {
	INIT_PLAYER_PACKET       = 1,
	SELF_NAME_PACKET         = 2,
	REMOVE_PLAYER_PACKET     = 3,
	STARTED_TYPING_PACKET    = 4,
	STOPPED_TYPING_PACKET    = 5,
	CHAT_MESSAGE_PACKET      = 6,
	CHAT_DISPLAY_NAME_PACKET = 7,
};

void sendPacket(ENetPeer* peer, PacketID packetID) {
    char packetData[256];
    snprintf(packetData, sizeof(packetData), "%d", packetID);
    ENetPacket* packet = enet_packet_create(packetData, strlen(packetData) + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
}

void sendPacket(ENetPeer* peer, PacketID packetID, const char* data) {
	char packetData[256];
	snprintf(packetData, sizeof(packetData), "%d %s", packetID, data);
	ENetPacket* packet = enet_packet_create(packetData, strlen(packetData) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}

void sendPacketInt(ENetPeer* peer, PacketID packetID, int data) {
	char packetData[256];
	snprintf(packetData, sizeof(packetData), "%d %i", packetID, data);
	ENetPacket* packet = enet_packet_create(packetData, strlen(packetData) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}

void sendPacketVec2(ENetPeer* peer, PacketID packetID, float x, float y) {
	char packetData[256];
	snprintf(packetData, sizeof(packetData), "%d %f %f", packetID, x, y);
	ENetPacket* packet = enet_packet_create(packetData, strlen(packetData) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}

void sendPacketVec3(ENetPeer* peer, PacketID packetID, float x, float y, float z) {
	char packetData[256];
	snprintf(packetData, sizeof(packetData), "%d %f %f %f", packetID, x, y, z);
	ENetPacket* packet = enet_packet_create(packetData, strlen(packetData) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}

void broadcastPacket(ENetHost* host, ENetPeer* sender, PacketID packetID) {
	ENetPeer* currentPeer;
	for (currentPeer = host->peers; currentPeer < &host->peers[host->peerCount]; ++currentPeer) {
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer == sender) {
			continue;
		}
		sendPacket(currentPeer, packetID);
	}
}

void broadcastPacket(ENetHost* host, ENetPeer* sender, PacketID packetID, const char* data) {
	ENetPeer* currentPeer;
	for (currentPeer = host->peers; currentPeer < &host->peers[host->peerCount]; ++currentPeer) {
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer == sender) {
			continue;
		}
		sendPacket(currentPeer, packetID, data);
	}
}

void broadcastPacketInt(ENetHost* host, ENetPeer* sender, PacketID packetID, int x) {
	ENetPeer* currentPeer;
	for (currentPeer = host->peers; currentPeer < &host->peers[host->peerCount]; ++currentPeer) {
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer == sender) {
			continue;
		}
		sendPacketInt(currentPeer, packetID, x);
	}
}

void broadcastPacketVec2(ENetHost* host, ENetPeer* sender, PacketID packetID, float x, float y) {
	ENetPeer* currentPeer;
	for (currentPeer = host->peers; currentPeer < &host->peers[host->peerCount]; ++currentPeer) {
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer == sender) {
			continue;
		}
		sendPacketVec2(currentPeer, packetID, x, y);
	}
}

void broadcastPacketVec3(ENetHost* host, ENetPeer* sender, PacketID packetID, float x, float y, float z) {
	ENetPeer* currentPeer;
	for (currentPeer = host->peers; currentPeer < &host->peers[host->peerCount]; ++currentPeer) {
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer == sender) {
			continue;
		}
		sendPacketVec3(currentPeer, packetID, x, y, z);
	}
}

// Handle specific packets recieved from client
void handlePacket(ENetPeer* peer, ENetPacket* packet) {
	int packetID;
	char data[256] = {};

	// Reading the packetID as an integer and the rest of the packet as string data
	int numArgs = sscanf_s((char*)packet->data, "%d %[^\t\n]", &packetID, data, (unsigned)_countof(data));

	switch (packetID) {
	case INIT_PLAYER_PACKET: 
	{
		printf("Init Player %s\n", Players[peer].c_str());
		printf("HIHIHI");

		broadcastPacket(peer->host, peer, INIT_PLAYER_PACKET, Players[peer].c_str());
        sendPacket(peer, SELF_NAME_PACKET, Players[peer].c_str());

		break;
	}
	case STARTED_TYPING_PACKET:
	{
		std::string user = Players[peer];
        broadcastPacket(peer->host, peer, STARTED_TYPING_PACKET, user.c_str());
		break;
	}
	case STOPPED_TYPING_PACKET:
	{
        std::string user = Players[peer];
        broadcastPacket(peer->host, peer, STOPPED_TYPING_PACKET, user.c_str());
		break;
	}
	case CHAT_MESSAGE_PACKET: 
	{
        std::string message(data);
        std::string user = Players[peer];
        std::string fullMessage = user + ": " + message;
        broadcastPacket(peer->host, peer, CHAT_MESSAGE_PACKET, fullMessage.c_str());
		break;
	}
	case CHAT_DISPLAY_NAME_PACKET:
	{
        std::string name(data);
        std::string oldName = Players[peer];
		Players[peer] = name;

        std::string nameChange = oldName + "," + name;
        broadcastPacket(peer->host, peer, CHAT_DISPLAY_NAME_PACKET, nameChange.c_str());
	}
	default:
	{
        printf("Unknown packet ID: %d\n", packetID);
	}
	}
}

int main(int argc, char** argv)
{
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}
	atexit(enet_deinitialize);

	ENetEvent event;
	ENetAddress address;
	ENetHost* server;

	/* Bind the server to the default localhost.     */
	/* A specific host address can be specified by   */
	/* enet_address_set_host (& address, "x.x.x.x"); */
	address.host = ENET_HOST_ANY; // This allows
	/* Bind the server to port 7777. */
	address.port = 7777;

	server = enet_host_create(&address	/* the address to bind the server host to */,
		32	/* allow up to 32 clients and/or outgoing connections */,
		1	/* allow up to 1 channel to be used, 0. */,
		0	/* assume any amount of incoming bandwidth */,
		0	/* assume any amount of outgoing bandwidth */);

	if (server == NULL)
	{
		printf("An error occurred while trying to create an ENet server host.");
		return 1;
	}

	printf("Server Started at port 7777!\n");

	// Packets recieved from client
	while (1) {
		while (enet_host_service(server, &event, 10) > 0) {
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT: {
				printf("Client %i connected.\n", idCounter);
				for (auto it : Players) {
					sendPacket(event.peer, INIT_PLAYER_PACKET, it.second.c_str());
                    // printf("Sending player %i to client %i\n", it.second, idCounter);
				}

				PushPlayer(event.peer);
				const char* name = Players[event.peer].c_str();
                for (auto it : Players) {
					if (it.first != event.peer) {
						sendPacket(it.first, INIT_PLAYER_PACKET, name);
					}
                }

				break;
			}
			case ENET_EVENT_TYPE_RECEIVE: {
				//printf("Received a packet from %i.\n", Players[event.peer]);
				handlePacket(event.peer, event.packet);
				enet_packet_destroy(event.packet);
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT: {
				printf("Client %i disconnected.\n", Players[event.peer]);
				broadcastPacket(event.peer->host, event.peer, REMOVE_PLAYER_PACKET, Players[event.peer].c_str());
				PopPlayer(event.peer);
				break;
			}
			}
		}
	}

	enet_host_destroy(server);
	enet_deinitialize();

	return 0;
}