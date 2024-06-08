#pragma once
#include <vector>
#include "Player.h"
#include "Flag.h"
using namespace std;

class GameState
{
	vector<Flag> flags;
	vector<Player> players;
	double timer;
	vector<int> score;
	bool isGameOver;
public:
	string serialize();
};

