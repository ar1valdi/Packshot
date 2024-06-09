#include "Client.h"
#include "Windows.h"
#include "Consts.h"
#include <thread>
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace std;

char Client::getPressedKey() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetForegroundWindow() != GetConsoleWindow()) {
        return NO_INPUT;
    }

    if ((GetAsyncKeyState('W') | GetAsyncKeyState(VK_UP)) & 0x8000) {
        return UP;
    }
    if ((GetAsyncKeyState('A') | GetAsyncKeyState(VK_LEFT)) & 0x8000) {
        return LEFT;
    }
    if ((GetAsyncKeyState('S') | GetAsyncKeyState(VK_DOWN)) & 0x8000) {
        return DOWN;
    }
    if ((GetAsyncKeyState('D') | GetAsyncKeyState(VK_RIGHT)) & 0x8000) {
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
    if (!connect()) {
        cerr << "Can't connect to the server\n";
        return;
    }
    mainLoop();
}

bool Client::connect() {
    running = connection.connectToServer();
    if (!running) {
        return false;
    }

    Action a;
    a.actionCode = NEW_PLAYER;
    gamestate = connection.sendToServer(a.serialize());
    id = gamestate.players.size() - 1;
    myPlayer = &gamestate.players[id];
    gotoxy(1, 25);
    cout << "moje id (:) " << id << '\n';

    Sleep(100);
    return true;
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
        for (auto& symbol : row) {
            if (symbol == 'F') {
                symbol = ' ';
            }
            if (symbol == '1' || symbol == '2' || symbol == '3' || symbol == '4') {
                symbol = ' ';
            }
        }
        newMap.push_back(row);
    }

    file.close();
    return newMap;
}

Client::Client() {
    running = true;
    direction = RIGHT;

    map = loadMap("map.txt");
    gotoxy(1, 20);
}

void Client::mainLoop() {
    fetch();
    draw();

    thread input(&Client::handleInputAsync, this);
    thread fetchThread(&Client::fetchAsync, this);
    unique_lock<mutex> lock(mapChange, defer_lock);

    gotoxy(1, 21);

    while (running) {
        lock.lock();
        draw();
        lock.unlock();
        gotoxy(1, 22);
        Sleep(50);
    }

    input.join();
    fetchThread.join();
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

    sendToServer(input);
}

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
            if (!myPlayer->isAlive) {
                break;
            }
            gotoxy(1, 23);
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

    gotoxy(0, 15);
    cout << "moved to " << x + dx << ", " << y + dy << '\n';

    map[y + dy][x + dx] = '@';
    myPlayer->position = { x + dx, y + dy };
}

ActionCode Client::inputToActionCode(char input) {
    switch (input) {
    case UP:
        return ActionCode(MOVE_UP);
    case DOWN:
        return ActionCode(MOVE_DOWN);
    case RIGHT:
        return ActionCode(MOVE_RIGHT);
    case LEFT:
        return ActionCode(MOVE_LEFT);
    case ATTACK_MOVE:
        return ActionCode(ATTACK);
    }
    return ActionCode(PRESENT);
}

void Client::sendToServer(char input) {
    ActionCode actionCode = inputToActionCode(input);
    Action action;
    action.actionCode = actionCode;
    action.playerID = id;
    string serializedAction = action.serialize();

    unique_lock<mutex> lock(fetching);
    GameState newGameState = connection.sendToServer(serializedAction);
    lock.unlock();

    update(newGameState);
}

void Client::fetch() {
    try {
        unique_lock<mutex> lock(fetching);
        GameState newGameState = connection.fetch();
        lock.unlock();
        update(newGameState);
    }
    catch (exception& e) {
        cerr << "Error fetching game state: " << e.what() << endl;
    }
}

void Client::fetchAsync() {
    while (running) {
        fetch();
        this_thread::sleep_for(chrono::milliseconds(100));  // Adjust as needed
    }
}

void Client::update(GameState& newGameState) {
    unique_lock<mutex> lock(mapChange);
    for (auto& player : gamestate.players) {
        int x = player.position.x;
        int y = player.position.y;

        if (player.isAlive) {
            map[y][x] = ' ';
        }
    }

    for (auto& player : newGameState.players) {
        int x = player.position.x;
        int y = player.position.y;

        if (player.isAlive) {
            map[y][x] = '@';
        }
    }

    gamestate = newGameState;
    myPlayer = &gamestate.players[id];

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