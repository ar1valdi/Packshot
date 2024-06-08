#pragma once
#include "GameState.h"
#include "Action.h"

class Game
{
	GameState gs;
	Game();
public:
	GameState handleIngameRequest(Action a);
	string getSerializedGameState();
};

