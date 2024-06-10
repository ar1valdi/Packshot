#include <iostream>
#include "Connection.h"
#include "Player.h"
#include <windows.h>
#include <conio.h>
#include "Consts.h"
#include "Client.h"

void testy_janek() {
    Connection c;
    c.connectToServer();

    
}

int main()
{
    //testy_janek();
    Client client;
    client.start();
    int x;
    cin >> x;
}

