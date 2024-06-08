#include <iostream>
#include "Server.h"

int main()
{
	Player p("Jan", 1, 2, true, 3, { 4,4 }, { 5,5 }, 6);
	string s = p.serialize();
	Player p2 = Player::deserialize(s);
	int x = 10;
	//Server server("127.0.0.1", 12345);
	//server.start();
}