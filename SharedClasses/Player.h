#include <string>
#include "Position.h"

class Player {
private:
    std::string name;
    int score;
    double deathTimer;
    bool isAlive;
    int attackRange;
    Position positions;
    Position spawnPoint;
    int numOfFlags;

public:
    Player() = default;
    ~Player() = default;
    Player(const Player&) = default;
    Player& operator=(const Player&) = default;

    Player(const std::string& name, int score, double deathTimer, bool isAlive,
        int attackRange, const Position& positions, const Position& spawnPoint,
        int numOfFlags)
        : name(name), score(score), deathTimer(deathTimer), isAlive(isAlive),
        attackRange(attackRange), positions(positions), spawnPoint(spawnPoint),
        numOfFlags(numOfFlags)
    {
    }

    std::string getName() const { return name; }
    int getScore() const { return score; }
    double getDeathTimer() const { return deathTimer; }
    bool getIsAlive() const { return isAlive; }
    int getAttackRange() const { return attackRange; }
    Position getPositions() const { return positions; }
    Position getSpawnPoint() const { return spawnPoint; }
    int getNumOfFlags() const { return numOfFlags; }

    void setName(const std::string& newName) { name = newName; }
    void setScore(int newScore) { score = newScore; }
    void setDeathTimer(double newDeathTimer) { deathTimer = newDeathTimer; }
    void setIsAlive(bool newIsAlive) { isAlive = newIsAlive; }
    void setAttackRange(int newAttackRange) { attackRange = newAttackRange; }
    void setPositions(const Position& newPosition) { positions = newPosition; }
    void setSpawnPoint(const Position& newSpawnPoint) { spawnPoint = newSpawnPoint; }
    void setNumOfFlags(int newNumOfFlags) { numOfFlags = newNumOfFlags; }
};
