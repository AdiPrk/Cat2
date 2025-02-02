#pragma once

namespace Dog
{

    class PlayerManager {
    public:
        PlayerManager();
        ~PlayerManager();

        // Adds a new player for the given peer.
        std::string pushPlayer(ENetPeer* peer);

        // Removes the player for the given peer.
        void popPlayer(ENetPeer* peer);

        // Returns the player’s name (or ID) for the given peer.
        std::string getPlayerName(ENetPeer* peer) const;

        // Updates the player’s name for the given peer.
        void updatePlayerName(ENetPeer* peer, const std::string& newName);

        // Returns a constant reference to the players map.
        const std::unordered_map<ENetPeer*, std::string>& getPlayers() const;

    private:
        std::unordered_map<ENetPeer*, std::string> players;
        int idCounter;
    };

}