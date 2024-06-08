#include <iostream>
#include "Connection.h"
#include "Player.h"
#include <windows.h>
#include <conio.h>
#include "Consts.h"
#include "Client.h"

	a.actionCode = ActionCode::MOVE_LEFT;
	gs = c.sendToServer(a.serialize());
	cout << gs.timer << "\n";

	gs = c.fetch();
	cout << gs.timer << "\n";

	Sleep(100000);
}

int main()
{
    Client client;
    client.start();
}

