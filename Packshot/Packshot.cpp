#include <iostream>
#include "Server.h"

int main()
{
	Server server("127.0.0.1", 12345);
	server.start();
}