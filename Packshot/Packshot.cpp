#include <iostream>
#include "Server.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

void foo(int i) {
	cout << i;
}

int main()
{
	Server server("127.0.0.1", 12345);
	server.start();
	thread t(foo, 1);
}