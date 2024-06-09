#include "Game.h"
#include "Consts.h"
#include <fstream>

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
	loadMap("map.txt");
	m_updateThread = thread(&Game::update, this);
}

void Game::stop()
{
	m_running = false;
	if (m_updateThread.joinable()) {
		m_updateThread.join();
	}
}

void Game::loadMap(const string filename) {
	ifstream file(filename);
	playerPositions.reserve(4);

	string line;
	int lineNo = 0;
	while (getline(file, line)) {
		vector<char> row(line.begin(), line.end());
		int col = 0;
		for (auto& symbol : row) {
			if (symbol == 'F') {
				Position pos(col, lineNo);
				Flag newFlag(pos, 5000, 5000, -1);
				m_gameState.flags.push_back(newFlag);
			}
			if (symbol == '1' || symbol == '2' || symbol == '3' || symbol == '4') {
				playerPositions.push_back({ col, lineNo });
			}
			col++;
		}
		lineNo++;
	}

	file.close();
}

GameState Game::handleRequest(Action a) {
	m_gameState.timer = m_timer;

	if (a.actionCode == ActionCode::NEW_PLAYER) {
		int playerCount = m_gameState.players.size();
		m_gameState.players.push_back(Player(m_gameState.players.size(), "Bob", 0, 0.0, 
				true, 1.0, playerPositions[playerCount], playerPositions[playerCount], 0));

		return m_gameState;
	}

	cout << a.playerID;
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

void Game::update()
{
	while (m_running) {
		for (auto& flag : m_gameState.flags) {
			if (flag.ownerID == -1) {
				continue;
			}

			flag.capturingTimer += GAME_UPDATE_RATE;

			if (flag.capturingTimer % 1000 != 0) {
				continue;
			}

			for (auto& player : m_gameState.players) {
				player.score += FLAG_POINT_GAIN;
			}
		}

		for (auto& player : m_gameState.players) {
			if (player.deathTimer > 0) {
				player.deathTimer -= GAME_UPDATE_RATE;
				if (player.deathTimer <= 0) {
					player.isAlive = true;
					player.deathTimer = 0;
				}
			}
		}
		m_timer += GAME_UPDATE_RATE;
		this_thread::sleep_for(chrono::milliseconds(GAME_UPDATE_RATE));
	}
}
