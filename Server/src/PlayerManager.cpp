#include <PCH/pch.h>
#include "PlayerManager.h"

namespace Dog
{
    PlayerManager::PlayerManager() : idCounter(0) {}

    PlayerManager::~PlayerManager() {}

    std::string PlayerManager::pushPlayer(ENetPeer* peer) {
        players[peer] = std::to_string(idCounter++);
        return players[peer];
    }

    void PlayerManager::popPlayer(ENetPeer* peer) {
        auto it = players.find(peer);
        if (it != players.end()) {
            players.erase(it);
        }
    }

    std::string PlayerManager::getPlayerName(ENetPeer* peer) const {
        auto it = players.find(peer);
        if (it != players.end()) {
            return it->second;
        }
        return "";
    }

    void PlayerManager::updatePlayerName(ENetPeer* peer, const std::string& newName) {
        players[peer] = newName;
    }

    const std::unordered_map<ENetPeer*, std::string>& PlayerManager::getPlayers() const {
        return players;
    }
}
