#include "Server.h"
#include "Consts.h"
#include <WS2tcpip.h>
#include <iostream>
#include <algorithm>
#include <chrono>

using namespace std;

Server::Server(const string& address, int port) : address(address), port(port), quit(false) {
    threads.reserve(MAX_CLIENTS_NUM + 1);
    connectedClients.store(0);
    isInQueue.store(true);
}

bool Server::startListening(const string& address, int port) {
    WSADATA wsadata;

    int err = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (err != NO_ERROR) {
        cerr << "WSA init error: " << WSAGetLastError() << endl;
        return false;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Error creating server socket: " << WSAGetLastError() << endl;
        WSACleanup();
        return false;
    }

    sockaddr_in service;
    memset(&service, 0, sizeof(service));
    service.sin_family = AF_INET;
    InetPton(AF_INET, __TEXT("127.0.0.1"), &service.sin_addr.s_addr);
    service.sin_port = htons(port);

    if (bind(serverSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        cerr << "Error while binding: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    err = listen(serverSocket, SOMAXCONN);
    if (err == SOCKET_ERROR) {
        cerr << "Failed to start listening: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    return true;
}

void Server::handleClient(SOCKET s) {
    int queueID = qm.getNextID();
    char recvBuf[1024];
    int bytesRecv;
    auto lastReceivedTime = std::chrono::steady_clock::now();

    while (!quit) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(s, &readfds);

        timeval timeout;
        timeout.tv_sec = 5;  // Set timeout for select
        timeout.tv_usec = 0;

        int result = select(0, &readfds, NULL, NULL, &timeout);
        if (result > 0 && FD_ISSET(s, &readfds)) {

            bytesRecv = recv(s, recvBuf, sizeof(recvBuf), 0);
            if (bytesRecv == SOCKET_ERROR) {
                cerr << "recv failed: " << WSAGetLastError() << endl;
                break;
            }
            if (bytesRecv == 0) {
                cout << "Connection closed\n";
                break;
            }

            cout << "Received " << bytesRecv << " bytes: " << recvBuf << '\n';
            lastReceivedTime = std::chrono::steady_clock::now();

            string sendBuf;
            if (isInQueue.load()) {
                int retflag;
                handleClientInQueue(recvBuf, sendBuf, s, retflag, queueID);
                if (retflag == 2) break;
            }
            else {
                if (!handleClientInGame(recvBuf, sendBuf, s)) {
                    break;
                }

                int retflag;
                sendWithLog(s, sendBuf, retflag);
                if (retflag == 2) break;
            }
        }
        else if (result == SOCKET_ERROR) {
            cerr << "select failed: " << WSAGetLastError() << endl;
            break;
        }

        auto currentTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastReceivedTime).count();
        if (duration > CLIENT_CUTOFF_TIME) {
            cout << "Client timed out\n";
            break;
        }
    }
    if (isInQueue.load()) {
        qm.erasePlayer(queueID);
    }

    connectedClients.store(connectedClients.load() - 1);
    closesocket(s);
    {
        lock_guard<mutex> lock(clientSocketsMutex);
        clientSockets.erase(remove(clientSockets.begin(), clientSockets.end(), s), clientSockets.end());
    }
    cout << "Closing thread " << this_thread::get_id() << endl;
}

void Server::handleClientInQueue(char  recvBuf[1024], std::string& sendBuf, SOCKET& s, int& retflag, int qID)
{
    retflag = 1;
    if (strcmp(recvBuf, FETCH_QUEUE) == 0) {
        sendBuf = qm.serializePlayerStates();
    }
    else {
        Action a = Action::deserialize(recvBuf);
        a.playerID = qID;
        qm.processAction(a);
        sendBuf = qm.serializePlayerStates();
    }

    int retflagIn;
    sendWithLog(s, sendBuf, retflagIn);
    if (retflagIn == 2) { retflag = 2; return; };

    if (qm.allPlayersReady()) {
        sendBuf = START_GAME;
        isInQueue.store(false);
    }
    else {
        sendBuf = STILL_QUEUE;
    }

    sendWithLog(s, sendBuf, retflagIn);
    if (retflagIn == 2) { retflag = 2; return; };
}

void Server::sendWithLog(const SOCKET& s, std::string& sendBuf, int& retflag)
{
    retflag = 1;
    int bytesSent = send(s, sendBuf.c_str(), sendBuf.length() + 1, 0);
    if (bytesSent == SOCKET_ERROR) {
        cerr << "send failed: " << WSAGetLastError() << endl;
        { retflag = 2; return; };;
    }
    cout << "Sent " << bytesSent << " bytes: " << sendBuf << '\n';
}

bool Server::handleClientInGame(char recvBuf[1024], string& sendBuf, SOCKET& s)
{
    if (strcmp(recvBuf, FETCH_QUEUE) == 0) {
        sendBuf = START_GAME;
        int retflag = 0;
        string q = qm.serializePlayerStates();
        sendWithLog(s, q, retflag);
        if (retflag == 2) { return false; }
        sendBuf = START_GAME;
    } else if (strcmp(recvBuf, FETCH_MSG) == 0) {
        sendBuf = game->getSerializedGameState();
    }
    else {
        Action a = Action::deserialize(recvBuf);
        GameState gs = game->handleRequest(a);
        sendBuf = gs.serialize();
    }
    return true;
}

void Server::runListenThread() {
    while (!quit) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);

        timeval timeout;
        timeout.tv_sec = 5;  // Set timeout for select
        timeout.tv_usec = 0;

        int result = select(0, &readfds, NULL, NULL, &timeout);
        if (result > 0 && FD_ISSET(serverSocket, &readfds)) {
            SOCKET newSocket = accept(serverSocket, NULL, NULL);

            if (newSocket == INVALID_SOCKET) {
                int err = WSAGetLastError();
                if (err != WSAEWOULDBLOCK && err != WSAECONNRESET) {
                    cerr << "accept failed: " << err << endl;
                }
                continue;
            }

            if (connectedClients.load() >= MAX_CLIENTS_NUM || !isInQueue.load()) {
                string sendBuf = NOT_ENOUGHT_SLOTS;
                int bytesSent = send(newSocket, sendBuf.c_str(), sendBuf.length() + 1, 0);
                continue;
            }
            string sendBuf = CONNECTED_CLIENT;
            int bytesSent = send(newSocket, sendBuf.c_str(), sendBuf.length() + 1, 0);

            cout << "Client connected\n";
            {
                lock_guard<mutex> lock(clientSocketsMutex);
                clientSockets.push_back(newSocket);
            }

            connectedClients.store(connectedClients.load() + 1);
            threads.emplace_back(&Server::handleClient, this, newSocket);
        }
        else if (result == SOCKET_ERROR) {
            cerr << "select failed: " << WSAGetLastError() << endl;
            break;
        }
    }
}

void Server::start() {
    game = new Game();
    game->start();

    if (!startListening(address, port)) {
        return;
    }

    threads.emplace_back(&Server::runListenThread, this);
    cout << "Enter 'quit' to shutdown server\n";
    while (!quit) {
        string comm;
        cin >> comm;
        if (comm == "quit") {
            quit = true;
        }
    }

    game->stop();

    closesocket(serverSocket);
    cout << "Waiting for threads to close themselves\n";
    cleanUpThreads();
    cout << "Threads closed\n";
    {
        lock_guard<mutex> lock(clientSocketsMutex);
        clientSockets.clear();
    }
    WSACleanup();
    cout << "Server closed\n";
}

void Server::cleanUpThreads() {
    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }
    threads.clear();
}