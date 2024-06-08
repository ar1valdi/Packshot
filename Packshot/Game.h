#pragma once

#include <atomic>
#include <thread>

#include "GameState.h"
#include "Action.h"

class Game
{
public:
	Game();
	~Game();

	GameState handleRequest(Action a);
	string getSerializedGameState();

	void start();
	void stop();

private:
	GameState m_gameState;
	int m_timer;

	thread m_updateThread;
	atomic<bool> m_running;

private:
	void handleFlags(Player player);
	void handleAttack(Player player);

	void updateClient();
};

