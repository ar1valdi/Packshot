#pragma once
#include <WinSock2.h>
#include "GameState.h"
#include <string>
using namespace std;

class Connection
{
private:
	SOCKET s;
public:
	Connection(); 
	bool connectToServer();
	bool connectToServer(const string& address, int port);
	GameState sendToServer(const string& req);
	pair<string, bool> sendToServerQueue(const string& req);
	GameState fetch();
	pair<string, bool> fetchQueue();
	~Connection();
};

