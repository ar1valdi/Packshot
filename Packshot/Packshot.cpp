#include <iostream>
#include "Server.h"

void testy_janek() {
	Flag f1({ 1,1 }, 2.3456, 7.8912, 3);
	Flag f2({ 2,2 }, 3.4567, 8.9123, 4);
	Player p1(12, "Jan1", 1, 2, true, 3, { 4,4 }, { 5,5 }, 6);
	Player p2(13, "Jan2", 11, 22, false, 33, { 44,44 }, { 55,55 }, 66);
	GameState gs;
	gs.flags = { f1, f2 };
	gs.players = { p1, p2 };
	gs.isGameOver = false;
	gs.score = { 0, 50 };
	gs.timer = 184.297134;
	string s = gs.serialize(1);
	GameState gs2 = GameState::deserialize(s);
	int x = 10;
	//Server server("127.0.0.1", 12345);
	//server.start();
}

int main()
{
	testy_janek();
}