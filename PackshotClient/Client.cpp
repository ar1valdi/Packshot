#include "Client.h"
#include "Windows.h"
#include "Consts.h"
#include <thread>
#include <fstream>
#include <iostream>
#include "Action.h"
#include <mutex>

using namespace std;

mutex mapChange;

char Client::getPressedKey() {
    if (GetAsyncKeyState('W') | GetAsyncKeyState(VK_UP) & 0x8000) {
        return UP;
    }
    if (GetAsyncKeyState('A') | GetAsyncKeyState(VK_LEFT) & 0x8000) {
        return LEFT;
    }
    if (GetAsyncKeyState('S') | GetAsyncKeyState(VK_DOWN) & 0x8000) {
        return DOWN;
    }
    if (GetAsyncKeyState('D') | GetAsyncKeyState(VK_RIGHT) & 0x8000) {
        return RIGHT;
    }
    if (GetAsyncKeyState(ESCAPE) & 0x8000) {
        return ESCAPE;
    }
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        return ATTACK_MOVE;
    }
    return NO_INPUT;
}

void Client::start() {
    connect();
    mainLoop();
}

void Client::connect() {
    running = connection.connectToServer();

    Action a;
    a.actionCode = NEW_PLAYER;
    gamestate = connection.sendToServer(a.serialize());

    Sleep(100);
}

vector<vector<char>> Client::loadMap(const string filename) {
    vector<vector<char>> newMap;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return newMap;
    }

    string line;
    while (getline(file, line)) {
        vector<char> row(line.begin(), line.end());
        newMap.push_back(row);
    }

    file.close();
    return newMap;
}

Client::Client() {
    running = true;
    direction = RIGHT;

    map = loadMap("map.txt");
}

void Client::mainLoop() {
    cout << "fetching\n";
    fetch();
    draw();

    thread input(&Client::handleInputAsync, this);
    unique_lock<mutex> lock(mapChange, defer_lock);

    while (running) {
        fetch();
        lock.lock();
        draw();
        lock.unlock();
        Sleep(10);
    }

    input.join();
}

void Client::makeAction(char input, char lastInput) {
    if (lastInput == input) {
        return;
    }
    if (!validateInput(input)) {
        return;
    }

    unique_lock<mutex> lock(mapChange);
    performPreAction(input);
    lock.unlock();

    /*auto sendThreadFunc = [this, input]() {
        sendToServer(input);
    };*/
    //thread send(sendThreadFunc);
    sendToServer(input);
}

// TODO 
void Client::handleInputAsync() {
    char input = NO_INPUT;
    char lastInput = NO_INPUT;

    while (running) {

        lastInput = input;
        input = getPressedKey();

        switch (input) {
        case ESCAPE:
            running = false;
            break;
        case NO_INPUT:
            break;
        default:
            makeAction(input, lastInput);
            break;
        }
    }
}

bool Client::validateInput(char input) {
    int x = myPlayer->position.x;
    int y = myPlayer->position.y;

    switch (input) {
    case UP:
        if (map[y - 1][x] != ' ') {
            return false;
        }
        break;
    case DOWN:
        if (map[y + 1][x] != ' ') {
            return false;
        }
        break;
    case RIGHT:
        if (map[y][x + 1] != ' ') {
            return false;
        }
        break;
    case LEFT:
        if (map[y][x - 1] != ' ') {
            return false;
        }
        break;
    }
    return true;
}

void Client::attack() {

}

// TODO
void Client::performPreAction(char input) {
    int x = myPlayer->position.x;
    int y = myPlayer->position.y;
    int dx = 0;
    int dy = 0;
    map[y][x] = ' ';

    if (input == UP) {
        dy = -1;
        direction = UP;
    }
    if (input == DOWN) {
        dy = 1;
        direction = DOWN;
    }
    if (input == RIGHT) {
        dx = 1;
        direction = RIGHT;
    }
    if (input == LEFT) {
        dx = -1;
        direction = LEFT;
    }
    if (input == ATTACK_MOVE) {
        attack();
    }

    map[y + dy][x + dx] = '@';
    myPlayer->position = { x + dx, y + dy };
}

ActionCode Client::inputToActionCode(char input) {
    switch (input) {
    case UP:
        return ActionCode(MOVE_UP);
        break;
    case DOWN:
        return ActionCode(MOVE_DOWN);
        break;
    case RIGHT:
        return ActionCode(MOVE_RIGHT);
        break;
    case LEFT:
        return ActionCode(MOVE_LEFT);
        break;
    case ATTACK_MOVE:
        return ActionCode(ATTACK);
        break;
    }
    return ActionCode(PRESENT);
}

void Client::sendToServer(char input) {
    ActionCode actionCode = inputToActionCode(input);
    Action action;
    action.actionCode = actionCode;
    string serializedAction = action.serialize();
    GameState newGameState = connection.sendToServer(serializedAction);

    update(newGameState);
}

// TODO
void Client::fetch() {
    GameState newGameState = connection.fetch();
    update(newGameState);
}

// TODO
void Client::update(GameState& newGameState) {
    unique_lock<mutex> lock(mapChange);
    for (auto player : gamestate.players) {
        int x = player.position.x;
        int y = player.position.y;

        if (player.isAlive) {
            map[y][x] = ' ';
        }
    }

    for (auto player : newGameState.players) {
        int x = player.position.x;
        int y = player.position.y;

        if (player.isAlive) {
            map[y][x] = '@';
        }
    }

    gamestate = newGameState;
    myPlayer = &gamestate.players[0]; // JUST FOR NOW

    lock.unlock();
}

void Client::clearScreen() {
    system("cls");
}

int Client::symbolToColor(char symbol) {
    switch (symbol) {
    case ' ':
        return 32;
    case 'W':
        return BACKGROUND_RED | BACKGROUND_GREEN;
    case '@':
        return BACKGROUND_RED;
    }
    return 6;
}

void Client::gotoxy(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = { x, y };
    SetConsoleCursorPosition(hConsole, coord);
}

// TODO
void Client::draw() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = { 0, 0 };
    SetConsoleCursorPosition(hConsole, coord);

    int rows = map.size();
    int cols = map[0].size();

    int graphicCols = 2 * cols;

    CHAR_INFO* buffer = new CHAR_INFO[rows * graphicCols];
    for (size_t y = 0; y < rows; y++) {
        for (size_t x = 0; x < graphicCols; x++) {
            int index = y * graphicCols + x;
            //buffer[index].Char.AsciiChar = map[y][x / (graphicCols / cols)];
            buffer[index].Char.AsciiChar = ' ';

            int color = symbolToColor(map[y][x / (graphicCols / cols)]);
            buffer[index].Attributes = color;
        }
    }

    SMALL_RECT rect = { 0, 0, graphicCols - 1, rows - 1 };
    COORD zeroCoord = { 0, 0 };
    COORD bufferSize = { graphicCols, rows };
    WriteConsoleOutputA(hConsole, buffer, bufferSize, zeroCoord, &rect);

    delete[] buffer;
}
