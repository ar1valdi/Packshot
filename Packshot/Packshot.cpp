#include <iostream>
#include "Server.h"

void testy_janek() {
	Server s("127.0.0.1", 12345);
	s.start();
}

int main()
{
	testy_janek();
}