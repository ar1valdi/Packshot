#include "GameState.h"

GameState::GameState() {
    this->timer = 0;
    this->isGameOver = false;
}

string GameState::serialize() {
    stringstream ss;

    ss << flags.size() << " ";
    for (Flag& f : flags) {
        ss << f.serialize() << " ";
    }

    ss << players.size() << " ";
    for (Player& p : players) {
        ss << p.serialize() << " ";
    }

    ss << timer << " ";

    ss << score.size() << " ";
    for (int s : score) {
        ss << s << " ";
    }

    ss << isGameOver;

    return ss.str();
}

GameState GameState::deserialize(const string& ser) {
    GameState gameState;
    stringstream ss(ser);

    size_t flagsSize;
    ss >> flagsSize;
    gameState.flags.resize(flagsSize);
    for (size_t i = 0; i < flagsSize; ++i) {
        string flagStr, temp;
        ss >> flagStr;
        for (int i = 0; i < Flag::fieldsToSerialize - 1; i++) {
            ss >> temp;
            flagStr += " " + temp;
        }
        gameState.flags[i] = Flag::deserialize(flagStr);
    }

    size_t playersSize;
    ss >> playersSize;
    gameState.players.resize(playersSize);
    for (size_t i = 0; i < playersSize; ++i) {
        string playerStr, temp;
        ss >> playerStr;
        for (int i = 0; i < Player::fieldsToSerialize-1; i++) {
            ss >> temp;
            playerStr += " " + temp;
        }
        gameState.players[i] = Player::deserialize(playerStr);
    }

    ss >> gameState.timer;

    size_t scoreSize;
    ss >> scoreSize;
    gameState.score.resize(scoreSize);
    for (size_t i = 0; i < scoreSize; ++i) {
        ss >> gameState.score[i];
    }

    ss >> gameState.isGameOver;

    if (ss.fail()) {
        throw runtime_error("Failed to deserialize GameState");
    }

    return gameState;
}