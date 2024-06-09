#include "Connection.h"
#include <WS2tcpip.h>
#include <iostream>
#include <thread>
#include "Consts.h"
using namespace std;

Connection::Connection() {
	s = INVALID_SOCKET;
}

bool Connection::connectToServer() {
	return connectToServer("127.0.0.1", 12345);
}

bool Connection::connectToServer(const string& address, int port) {
	WSADATA wsadata;

	int err = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (err != NO_ERROR) {
		cerr << "WSA init error: " << WSAGetLastError() << endl;
		return false;
	}

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		cerr << "Error creating client socket: " << WSAGetLastError() << endl;
		WSACleanup();
		return false;
	}

	sockaddr_in service;
	memset(&service, 0, sizeof(service));
	service.sin_family = AF_INET;
	InetPton(AF_INET, __TEXT("127.0.0.1"), &service.sin_addr.s_addr);
	service.sin_port = htons(port);

	if (connect(s, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
		cerr << "Failed to connect: " << WSAGetLastError() << endl;
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
	if (bytesSent <= 0) {
		closesocket(s);
		WSACleanup();
		throw runtime_error("send failed: " + to_string(WSAGetLastError()));
	}

	//cout << "Bytes sent: " << bytesSent << '\n';

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(s, &readfds);

	timeval timeout;
	timeout.tv_sec = 5;  // Set timeout for select
	timeout.tv_usec = 0;

	int bytesRecv = SOCKET_ERROR;
	char recvBuf[CLIENT_RECV_BUF] = { 0 };

	if (select(0, &readfds, NULL, NULL, &timeout) > 0) {
		bytesRecv = recv(s, recvBuf, CLIENT_RECV_BUF, 0);
		if (bytesRecv == 0) {
			closesocket(s);
			WSACleanup();
			throw runtime_error("connection closed");
		}
		else if (bytesRecv < 0) {
			closesocket(s);
			WSACleanup();
			throw runtime_error("recv failed: " + to_string(WSAGetLastError()));
		}
		//cout << "Received " << bytesRecv << " bytes: " << recvBuf << '\n';
	}
	else {
		closesocket(s);
		WSACleanup();
		throw runtime_error("recv timeout");
	}

	return GameState::deserialize(recvBuf);
}

GameState Connection::fetch() {
	GameState newGameState = sendToServer(FETCH_MSG);
	return newGameState;
}

Connection::~Connection() {
	if (s != INVALID_SOCKET) {
		closesocket(s);
		WSACleanup();
	}
}