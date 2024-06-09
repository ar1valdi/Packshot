#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <WinSock2.h>
#include "GameState.h"
#include "Action.h"
#include "Game.h"

#define MAX_CLIENTS_NUM 10

class Server {
public:
    Server(const std::string& address, int port);
    void start();
    void stop();

private:
    std::atomic<bool> isInQueue;
    std::string address;
    int port;
    SOCKET serverSocket;
    std::atomic<bool> quit;
    Game* game;
    std::vector<std::thread> threads;
    std::vector<SOCKET> clientSockets;
    std::mutex clientSocketsMutex;
    std::atomic<int> connectedClients;

    bool startListening(const std::string& address, int port);
    void handleClient(SOCKET s);
    void runListenThread();
    void cleanUpThreads();
};