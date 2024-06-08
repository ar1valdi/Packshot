#pragma once
#include "GameState.h"
#include "Action.h"

class Game
{
	Game();
public:
	GameState handleIngameRequest(Action a);
};

