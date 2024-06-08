#pragma once
#include <string>
#include "Position.h"

class Player {
private:
    int id;
    std::string name;
    int score;
    double deathTimer;
    bool isAlive;
    int attackRange;
    Position position;
    Position spawnPoint;
    int numOfFlags;

public:
    Player() = default;
    ~Player() = default;
    Player(const Player&) = default;
    Player& operator=(const Player&) = default;

    Player(int id, const std::string& name, int score, double deathTimer, bool isAlive,
        int attackRange, const Position& position, const Position& spawnPoint,
        int numOfFlags)
        : id(id), name(name), score(score), deathTimer(deathTimer), isAlive(isAlive),
        attackRange(attackRange), position(position), spawnPoint(spawnPoint),
        numOfFlags(numOfFlags)
    {
    }

    int getId() const { return id; }
    std::string getName() const { return name; }
    int getScore() const { return score; }
    double getDeathTimer() const { return deathTimer; }
    bool getIsAlive() const { return isAlive; }
    int getAttackRange() const { return attackRange; }
    Position getPosition() const { return position; }
    Position getSpawnPoint() const { return spawnPoint; }
    int getNumOfFlags() const { return numOfFlags; }

    void setId(int newId) { id = newId; }
    void setName(const std::string& newName) { name = newName; }
    void setScore(int newScore) { score = newScore; }
    void setDeathTimer(double newDeathTimer) { deathTimer = newDeathTimer; }
    void setIsAlive(bool newIsAlive) { isAlive = newIsAlive; }
    void setAttackRange(int newAttackRange) { attackRange = newAttackRange; }
    void setPosition(const Position& newPosition) { position = newPosition; }
    void setSpawnPoint(const Position& newSpawnPoint) { spawnPoint = newSpawnPoint; }
    void setNumOfFlags(int newNumOfFlags) { numOfFlags = newNumOfFlags; }
    
    std::string serialize();
    static Player deserialize(const std::string& ser);
};
