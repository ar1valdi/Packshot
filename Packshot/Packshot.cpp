#include <iostream>
#include "Server.h"
#include "QueueManager.h"

void testy_janek() {
	Server s("127.0.0.1", 12345);
	s.start();
}

int main()
{
	//Action a;
	//a.actionCode = READY_IN_LOBBY;
	//a.playerID = 1;
	//
	//QueueManager qm;
	//qm.addPlayer(1);
	//qm.addPlayer(2);
	//qm.addPlayer(3);
	//cout << qm.serializePlayerStates() << '\n';
	//qm.processAction(a);
	//qm.setPlayerState(2, true);
	//cout << qm.serializePlayerStates() << '\n';
	//cout << qm.allPlayersReady() << '\n';
	//qm.setPlayerState(3, true);
	//cout << qm.serializePlayerStates() << '\n';
	//cout << qm.allPlayersReady() << '\n';

	Server s("127.0.0.1", 12345);
	s.start();
}