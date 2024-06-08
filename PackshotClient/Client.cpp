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
    if (GetAsyncKeyState(UP) & 0x8000) {
        return UP;
    }
    if (GetAsyncKeyState(LEFT) & 0x8000) {
        return LEFT;
    }
    if (GetAsyncKeyState(DOWN) & 0x8000) {
        return DOWN;
    }
    if (GetAsyncKeyState(RIGHT) & 0x8000) {
        return RIGHT;
    }
    if (GetAsyncKeyState(ESCAPE) & 0x8000) {
        return ESCAPE;
    }
    return NO_INPUT;
}

void Client::start() {
    //connect();
    mainLoop();
}

void Client::connect() {
    running = connection.connectToServer();
    gamestate = connection.sendToServer("Hello World - client");
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

    map = loadMap("map.txt");

    myPlayer.setPositions({1, 1});
    map[myPlayer.getPositions().x][myPlayer.getPositions().y] = '@';
}

void Client::mainLoop() {
    thread input(&Client::handleInputAsync, this);
    unique_lock<mutex> lock(mapChange, defer_lock);

    while (running) {
        fetch();
        lock.lock();
        draw();
        lock.unlock();
        Sleep(5);
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

    auto sendThreadFunc = [this, input]() {
        sendToServer(input);
    };
    //thread send(sendThreadFunc);
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
    int x = myPlayer.getPositions().x;
    int y = myPlayer.getPositions().y;

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

// TODO
void Client::performPreAction(char input) {
    int x = myPlayer.getPositions().x;
    int y = myPlayer.getPositions().y;
    int dx = 0;
    int dy = 0;
    map[y][x] = ' ';

    if (input == UP) {
        dy = -1;
    }
    if (input == DOWN) {
        dy = 1;
    }
    if (input == RIGHT) {
        dx = 1;
    }
    if (input == LEFT) {
        dx = -1;
    }

    map[y + dy][x + dx] = '@';
    myPlayer.setPositions({ x + dx, y + dy });
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
    }
    return ActionCode(PRESENT);
}

void Client::sendToServer(char input) {
    ActionCode actionCode = inputToActionCode(input);
    Action action;
    action.actionCode = actionCode;
    string serializedAction = action.serialize();
    GameState newGameState = connection.sendToServer(serializedAction);

    unique_lock<mutex> lock(mapChange);
    lock.lock();
    update(newGameState);
    lock.unlock();
}

// TODO
void Client::fetch() {
}

// TODO
void Client::update(GameState& newGameState) {
    
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
