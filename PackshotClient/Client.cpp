#include "Client.h"
#include "Windows.h"
#include "Consts.h"
#include <thread>
#include <iostream>

using namespace std;

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
    connect();
    mainLoop();
}

void Client::connect() {
    running = connection.connectToServer();
    gamestate = connection.sendToServer("Hello World - client");
    Sleep(100);
}

Client::Client() {
    running = true;

    int width = 10;
    int height = 10;
    map = vector<vector<char>>(height, vector<char>(width, 'A'));
}

void Client::mainLoop() {
    thread input(&Client::handleInputAsync, this);

    while (running) {
        fetch();
        draw();
        Sleep(500);
    }

    input.join();
}

// TODO 
void Client::handleInputAsync() {
    char input;

    while (running) {

        input = getPressedKey();

        switch (input) {
        case ESCAPE:
            running = false;
            break;
        case NO_INPUT:
            break;
        default:
            break;
        }

        Sleep(100);
    }
}

// TODO
void Client::validateInput() {
}

// TODO
void Client::performPreAction() {
}

// TODO
void Client::sendToServer() {
}

// TODO
void Client::fetch() {
}

// TODO
void Client::update() {
}

void Client::clearScreen() {
    system("cls");
}

// TODO
void Client::draw() {
    clearScreen();
    int rows = map.size();
    int cols = map[0].size();

    int graphicCols = 2 * cols;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CHAR_INFO* buffer = new CHAR_INFO[rows * graphicCols];
    for (size_t y = 0; y < rows; y++) {
        for (size_t x = 0; x < graphicCols; x++) {
            int index = y * graphicCols + x;
            buffer[index].Char.AsciiChar = map[y][x / graphicCols * cols];
            buffer[index].Attributes = 16;
        }
    }

    SMALL_RECT rect = { 0, 0, graphicCols - 1, rows - 1 };
    COORD zeroCoord = { 0, 0 };
    COORD bufferSize = { graphicCols, rows };
    WriteConsoleOutputA(hConsole, buffer, bufferSize, zeroCoord, &rect);

    delete[] buffer;
}
