#include <vector>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <sstream>
#include "Player.h"
#include "Action.h"

class QueueManager {
private:
    std::unordered_map<int, bool> playerStates;

public:
    bool allPlayersReady() const {
        return std::all_of(playerStates.begin(), playerStates.end(), [](const auto& pair) {
            return pair.second;
            });
    }

    void addPlayer(int playerId) {
        playerStates[playerId] = false;
    }

    void setPlayerState(int playerId, bool v) {
        if (playerStates.find(playerId) != playerStates.end()) {
            playerStates[playerId] = v;
        }
    }

    void erasePlayer(int playerId) {
        playerStates.erase(playerId);
    }

    std::vector<std::pair<int, bool>> getPlayerStates() const {
        std::vector<std::pair<int, bool>> states;
        for (const auto& pair : playerStates) {
            states.emplace_back(pair.first, pair.second);
        }
        return states;
    }

    std::string serializePlayerStates() const {
        std::ostringstream oss;
        for (const auto& pair : playerStates) {
            oss << pair.first << ":" << pair.second << ";";
        }
        return oss.str();
    }

    void processAction(const Action& a) {
        int id = a.playerID;
        bool v;
        switch (a.actionCode) {
        case ActionCode::READY_IN_LOBBY:
            v = true;
            break;
        case ActionCode::NOT_READY_IN_LOBBY:
            v = false;
            break;
        default:
            return;
        }
        setPlayerState(id, v);
    }
};