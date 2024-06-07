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
	bool connectToServer(string address, int port);
	bool sendToServer(string req);
	GameState fetch();
	~Connection();
};

