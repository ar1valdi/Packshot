#pragma once
#include "Player.h"
#include "GameState.h"
#include "Connection.h"
#include "Action.h"
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
	void makeAction(char input, char lastInput);
	bool validateInput(char input);
	void performPreAction(char input);
	void sendToServer(char input);
	void fetch();
	void update(GameState& newGameState);
	void draw();
	void connect();
	vector<vector<char>> loadMap(const string filename);

public:
	Client();
	~Client() = default;

	void start();

	static void clearScreen();
	static char getPressedKey();
	static int symbolToColor(char symbol);
	static ActionCode inputToActionCode(char input);
};