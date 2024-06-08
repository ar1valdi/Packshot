#pragma once
#include "Player.h"
#include "GameState.h"
#include "Connection.h"
#include <vector>

class Client {
private:
	Connection connection;
	Player myPlayer;
	std::vector<std::vector<char>> map;
	GameState gamestate;
	std::vector<int> scores;
	bool running;

	void mainLoop();
	void handleInputAsync();
	void validateInput();
	void performPreAction();
	void sendToServer();
	void fetch();
	void update();
	void draw();
	void connect();

public:
	Client();
	~Client() = default;

	void start();

	static void clearScreen();
	static char getPressedKey();
};