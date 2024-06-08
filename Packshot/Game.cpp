#include "Game.h"
#include "Consts.h"

Game::Game() 
	: m_timer(0), m_running(false)
{
}

Game::~Game()
{
	stop();
}

void Game::start()
{
	m_running = true;
	m_updateThread = thread(&Game::updateClient, this);
}

void Game::stop()
{
	m_running = false;
	if (m_updateThread.joinable()) {
		m_updateThread.join();
	}
}

GameState Game::handleRequest(Action a) {
	for (auto& player : m_gameState.players) {
		if (player.id != a.playerID) {
			continue;
		}

		switch (a.actionCode) {
			case ActionCode::MOVE_UP:
				player.position.y--;
				break;
			case ActionCode::MOVE_DOWN:
				player.position.y++;
				break;
			case ActionCode::MOVE_RIGHT:
				player.position.x++;
				break;
			case ActionCode::MOVE_LEFT:
				player.position.x--;
				break;
			case ActionCode::ATTACK:
				handleAttack(player);
				break;
		}

		handleFlags(player);
	}

	m_gameState.timer = m_timer;

	return m_gameState;
}

string Game::getSerializedGameState() {
	return m_gameState.serialize();
}

void Game::handleFlags(Player player)
{
	for (auto& flag : m_gameState.flags) {
		if (player.position != flag.position || player.id == flag.ownerID) {
			continue;
		}

		flag.ownerID = player.id;
		flag.capturingTimer = 0;
	}
}

void Game::handleAttack(Player player)
{
	for (auto& other : m_gameState.players) {
		if (other.position.x - 1 == player.position.x ||
			other.position.x + 1 == player.position.x ||
			other.position.y - 1 == player.position.y ||
			other.position.y + 1 == player.position.y)
		{
			other.isAlive = false;
			other.deathTimer = DEATH_COOLDOWN;
			other.position = other.spawnPoint;

			player.score += KILL_POINT_GAIN;
		}
	}
}

void Game::updateClient()
{
	while (m_running) {
		for (auto& flag : m_gameState.flags) {
			if (flag.ownerID == -1) {
				continue;
			}

			flag.capturingTimer += GAME_UPDATE_RATE;

			if (flag.capturingTimer % 1000 == 0) {
				for (auto& player : m_gameState.players) {
					player.score += FLAG_POINT_GAIN;
				}
			}
		}

		m_timer += GAME_UPDATE_RATE;
		this_thread::sleep_for(chrono::milliseconds(GAME_UPDATE_RATE));
	}
}
