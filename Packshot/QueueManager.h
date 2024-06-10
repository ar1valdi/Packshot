#pragma once
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <sstream>
#include "Player.h"
#include "Action.h"
#include <mutex>
#include "Consts.h"

class QueueManager {
private:
    std::unordered_map<int, bool> playerStates;
    std::mutex mtx;
    int nextID = 0;

public:

    int getNextID() {
        std::unique_lock<std::mutex> lock(mtx);
        nextID += 1;
        return nextID;
    }

    bool allPlayersReady() {
        std::unique_lock<std::mutex> lock(mtx);
        if (playerStates.size() < MIN_PLAYERS_TO_START_GAME) return false;
        return std::all_of(playerStates.begin(), playerStates.end(), [](const auto& pair) {
            return pair.second;
            });
    }

    void addPlayer(int playerId) {
        std::unique_lock<std::mutex> lock(mtx);
        playerStates[playerId] = false;
    }

    void setPlayerState(int playerId, bool v) {
        std::unique_lock<std::mutex> lock(mtx);
        if (playerStates.find(playerId) != playerStates.end()) {
            playerStates[playerId] = v;
        }
    }

    void erasePlayer(int playerId) {
        std::unique_lock<std::mutex> lock(mtx);
        playerStates.erase(playerId);
    }

    std::vector<std::pair<int, bool>> getPlayerStates(){
        std::unique_lock<std::mutex> lock(mtx);
        std::vector<std::pair<int, bool>> states;
        for (const auto& pair : playerStates) {
            states.emplace_back(pair.first, pair.second);
        }
        return states;
    }

    std::string serializePlayerStates() {
        std::unique_lock<std::mutex> lock(mtx);
        std::ostringstream oss;
        for (const auto& pair : playerStates) {
            oss << pair.first << ":" << pair.second << ";";
        }
        return oss.str();
    }

    void processAction(const Action& a) {
        int id = a.playerID;
        bool v;
        addPlayer(id);

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