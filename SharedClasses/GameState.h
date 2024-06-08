#pragma once
#include <vector>
#include "Player.h"
#include "Flag.h"
using namespace std;

class GameState
{
public:
	vector<Flag> flags;
	vector<Player> players;
	double timer;
	vector<int> score;
	bool isGameOver;

	GameState();
	string serialize();
	static GameState deserialize(const string& ser);
};

