#pragma once

#include <vector>
#include <string>
#include <mutex>
#include "GameState.h"
#include "Action.h"
#include "Connection.h"

class Client {
public:
    Client();
    void start();

private:
    bool running;
    char direction;
    int id;
    GameState gamestate;
    Player* myPlayer;
    std::vector<std::vector<char>> map;
    Connection connection;

    mutex mapChange;
    mutex fetching;

    char getPressedKey();
    void connect();
    std::vector<std::vector<char>> loadMap(const std::string filename);
    void mainLoop();
    void makeAction(char input, char lastInput);
    void handleInputAsync();
    bool validateInput(char input);
    void performPreAction(char input);
    void attack();
    ActionCode inputToActionCode(char input);
    void sendToServer(char input);
    void fetch();
    void fetchAsync();
    void update(GameState& newGameState);
    void clearScreen();
    int symbolToColor(char symbol);
    void gotoxy(int x, int y);
    void draw();
};