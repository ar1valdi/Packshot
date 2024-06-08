#include "Connection.h"
#include <WS2tcpip.h>
#include <iostream>
#include <thread>
#include "Consts.h"
using namespace std;

Connection::Connection() {
	s = SOCKET_ERROR;
}
bool Connection::connectToServer() {
	return connectToServer("127.0.0.1", 12345);
}
bool Connection::connectToServer(const string& address, int port) {
	WSADATA wsadata;

	int err = WSAStartup((2, 2), &wsadata);
	if (err != NO_ERROR) {
		cout << "WSA init error: " << WSAGetLastError();
		return false;
	}

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		cout << "Error creating client socket: " << WSAGetLastError();
		WSACleanup();
		return false;
	}

	sockaddr_in service;
	memset(&service, 0, sizeof(service));
	service.sin_family = AF_INET;
	InetPton(AF_INET, __TEXT("127.0.0.1"), &service.sin_addr.s_addr);
	service.sin_port = htons(port);

	if (connect(s, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
	{
		cout << "Failed to connect: " << WSAGetLastError();
		closesocket(s);
		WSACleanup();
		return false;
	}

	u_long mode = 1;
	ioctlsocket(s, FIONBIO, &mode);

	return true;
}
GameState Connection::sendToServer(const string& req) {
	int bytesSent = send(s, req.c_str(), req.length() + 1, 0);
	int bytesRecv = SOCKET_ERROR;
	char recvBuf[CLIENT_RECV_BUF];

	if (bytesSent <= 0) {
		closesocket(s);
		WSACleanup();
		throw new exception("send failed: " + WSAGetLastError());
	}

	//cout << "Bytes sent: " << bytesSent << '\n';
	while (bytesRecv == SOCKET_ERROR) {
		int err = WSAEWOULDBLOCK;
		while (err == WSAEWOULDBLOCK) {
			bytesRecv = recv(s, recvBuf, 1024, 0);
			this_thread::sleep_for(chrono::milliseconds(100));
			err = WSAGetLastError();
		}

		if (bytesRecv == 0) {
			closesocket(s);
			WSACleanup();
			throw new exception("connection closed");
		}
		else if (bytesRecv < 0) {
			closesocket(s);
			WSACleanup();
			throw new exception("recv failed: " + err);
		}
		//cout << "Recieved " << bytesRecv << " bytes: " << recvBuf << '\n';

	}

	return GameState::deserialize(recvBuf);
}
GameState Connection::fetch() {
	return sendToServer(FETCH_MSG);
}
Connection::~Connection() {
	closesocket(s);
	WSACleanup();
}