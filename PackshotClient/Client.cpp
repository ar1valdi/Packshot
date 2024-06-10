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

    Connection c;
    c.connectToServer();

    Action a;
    a.playerID = 1;
    a.actionCode = ActionCode::READY_IN_LOBBY;
    auto ret = c.sendToServerQueue(a.serialize());
    cout << ret.first << " " << ret.second;

    while (!ret.second) {
        auto ret = c.fetchQueue();
        cout << ret.first << " " << ret.second << '\n';
        Sleep(1000);
    }

   // Action a;
    a.actionCode = NEW_PLAYER;
    gamestate = connection.sendToServer(a.serialize());
    id = gamestate.players.size() - 1;
    myPlayer = &gamestate.players[id];

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
}

void Client::mainLoop() {
    fetch();
    draw();

    thread input(&Client::handleInputAsync, this);
    thread fetchThread(&Client::fetchAsync, this);
    unique_lock<mutex> lock(mapChange, defer_lock);


    while (running) {
        lock.lock();
        draw();
        lock.unlock();
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
    attacking = true;
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

    GameState newGameState;
    unique_lock<mutex> lock(fetching);
    try {
        newGameState = connection.sendToServer(serializedAction);
    }
    catch (exception& e) {
        if (strcmp(e.what(), "recv failed: 0") == 0) {
            cerr << "Lost connection to the server" << endl;
        }
        else {
            cerr << "Error fetching game state: " << e.what() << endl;
        }
        running = false;
        newGameState = gamestate;
    }
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
        if (strcmp(e.what(), "recv failed: 0") == 0) {
            cerr << "Lost connection to the server" << endl;
        }
        else {
            cerr << "Error fetching game state: " << e.what() << endl;
        }
        running = false;
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

    for (auto& flag : gamestate.flags) {
        int x = flag.position.x;
        int y = flag.position.y;
        int index = y * graphicCols + x * 2;

        buffer[index].Char.AsciiChar = ' ';
        buffer[index].Attributes = 48;
        buffer[index + 1].Char.AsciiChar = ' ';
        buffer[index + 1].Attributes = 48;
    }

    if (myPlayer->isAlive) {
        int x = myPlayer->position.x;
        int y = myPlayer->position.y;
        int index = y * graphicCols + x * 2;

        buffer[index].Char.AsciiChar = ' ';
        buffer[index].Attributes = 80;
        buffer[index + 1].Char.AsciiChar = ' ';
        buffer[index + 1].Attributes = 80;

        if (attacking) {
            buffer[index - 1].Char.AsciiChar = '-';
            buffer[index - 2].Char.AsciiChar = '-';
            buffer[index + 2].Char.AsciiChar = '-';
            buffer[index + 3].Char.AsciiChar = '-';
            buffer[(y - 1) * graphicCols + x * 2].Char.AsciiChar = '|';
            buffer[(y + 1) * graphicCols + x * 2].Char.AsciiChar = '|';
            buffer[(y - 1) * graphicCols + x * 2 + 1].Char.AsciiChar = '|';
            buffer[(y + 1) * graphicCols + x * 2 + 1].Char.AsciiChar = '|';
            attacking = false;
        }
    }

    SMALL_RECT rect = { 0, 0, graphicCols - 1, rows - 1 };
    COORD zeroCoord = { 0, 0 };
    COORD bufferSize = { graphicCols, rows };
    WriteConsoleOutputA(hConsole, buffer, bufferSize, zeroCoord, &rect);

    int scoresX = map[0].size() * 2 + 10;
    for (int i = 0; i < gamestate.players.size(); i++) {
        gotoxy(scoresX, i);
        string yourPlayer = i == myPlayer->id ? " (You)" : "";
        cout << "Player " << i + 1 << yourPlayer << ": " << gamestate.players[i].score;
    }

    delete[] buffer;
}