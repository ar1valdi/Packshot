#pragma once
#include "Game.h"
#include <vector>
#include <thread>
#include <winsock2.h>
#include <iostream>
#include <mutex>
#include "Consts.h"

using namespace std;

class Server {
private:
	volatile bool quit = false;
	int port;
	int connectedClients;
	string address;
	Game* game;
	SOCKET serverSocket;
	vector<SOCKET> clientSockets;
	pair<thread, bool> threads[MAX_CLIENTS_NUM + 1];
	mutex mtx;

	void runListenThread();
	bool startListening(const string& address, int port);
	void handleClient(SOCKET& s);
	void cleanClientThreadData(SOCKET& s, thread::id threadID);
	void addThread(SOCKET& s);
public:
	Server(const string& address, int port);
	void start();
	~Server();
};