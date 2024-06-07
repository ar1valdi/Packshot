#pragma once
#include "Game.h"
#include <vector>
#include <thread>
#include <winsock2.h>
#include <iostream>

using namespace std;

class Server {
private:
	volatile bool quit = false;
	int port;
	string address;
	Game* game;
	SOCKET serverSocket;
	vector<SOCKET> clientSockets;
	vector<thread> threads;

	void runListenThread();
	bool startListening(const string& address, int port);
	void handleClient(SOCKET& s);
public:
	Server(const string& address, int port);
	void start();
};