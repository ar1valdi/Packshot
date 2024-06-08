#pragma once
#include "GameState.h"
#include "Action.h"

class Game
{
	GameState gs;
public:
	Game();
	GameState handleIngameRequest(Action a);
	string getSerializedGameState();
};

