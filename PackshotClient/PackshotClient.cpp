#include <iostream>
#include "Connection.h"
#include "Action.h"

void testy_janek() {
	Action a;
	Connection c;
	GameState gs;
	c.connectToServer();

	a.actionCode = ActionCode::MOVE_UP;
	gs = c.sendToServer(a.serialize());
	cout << gs.timer << "\n";

	a.actionCode = ActionCode::MOVE_DOWN;
	gs = c.sendToServer(a.serialize());
	cout << gs.timer << "\n";

	a.actionCode = ActionCode::MOVE_RIGHT;
	gs = c.sendToServer(a.serialize());
	cout << gs.timer << "\n";

	a.actionCode = ActionCode::MOVE_LEFT;
	gs = c.sendToServer(a.serialize());
	cout << gs.timer << "\n";

	gs = c.fetch();
	cout << gs.timer << "\n";

	Sleep(100000);
}

int main()
{
	testy_janek();
}