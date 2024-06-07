#include <iostream>
#include "Server.h"

void foo(int i) {
	cout << i;
}

int main()
{
	Server server("127.0.0.1", 12345);
	server.start();
	thread t(foo, 1);
}