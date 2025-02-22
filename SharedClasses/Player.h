#pragma once
#include <string>
#include "Position.h"

class Player {
public:
    static const int fieldsToSerialize = 11;

    Player() = default;
    ~Player() = default;

    Player(const Player&) = default;
    Player& operator=(const Player&) = default;


    Player(int id, const std::string& name, int score, double deathTimer, bool isAlive,
        int attackRange, const Position& position, const Position& spawnPoint,
        int numOfFlags)
        : id(id), name(name), score(score), deathTimer(deathTimer), isAlive(isAlive),
        attackRange(attackRange), position(position), spawnPoint(spawnPoint),
        numOfFlags(numOfFlags), isAttacking(false)
    {
    }

    std::string serialize();
    static Player deserialize(const std::string& ser);

public:
    int id;
    int score;
    int attackRange;
    int numOfFlags;
    bool isAttacking;

    Position position;
    Position spawnPoint;

    std::string name;
    double deathTimer;
    bool isAlive;
};
