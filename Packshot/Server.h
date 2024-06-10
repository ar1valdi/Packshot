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
#include "QueueManager.h"

#define MAX_CLIENTS_NUM 10

class Server {
public:
    Server(const std::string& address, int port);
    void start();
    void stop();

private:
    QueueManager qm;
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
    void handleClientInQueue(char  recvBuf[1024], std::string& sendBuf, SOCKET& s, int& retflag, int qID);
    void sendWithLog(const SOCKET& s, std::string& sendBuf, int& retflag);
    void handleClientInGame(char  recvBuf[1024], string& sendBuf);
    void runListenThread();
    void cleanUpThreads();
};