#include "Server.h"
#include <functional>
using namespace std;

Server::Server(const string& address, int port) {
	this->address = address;
	this->port = port;
}
bool Server::startListening(const string& address, int port) {
	WSADATA wsadata;

	int err = WSAStartup((2, 2), &wsadata);
	if (err != NO_ERROR) {
		perror("WSA init error: ");
		return false;
	}

	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET) {
		perror("Error creating server socket: ");
		WSACleanup();
		return false;
	}

	sockaddr_in service;
	memset(&service, 0, sizeof(service));
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(address.c_str());
	service.sin_port = htons(port);

	bind(serverSocket, (SOCKADDR*)&service, sizeof(service));
	if (err == SOCKET_ERROR) {
		perror("Error while binding: ");
		closesocket(serverSocket);
		WSACleanup();
		return false;
	}

	err = listen(serverSocket, 1);
	if (err == SOCKET_ERROR) {
		perror("Error while starting listening: ");
		closesocket(serverSocket);
		WSACleanup();
		return false;
	}

	return true;
}
void Server::handleClient(SOCKET& s) {
	int bytesSent;
	int bytesRecv = SOCKET_ERROR;
	char sendBuf[32] = "Hello world - server";
	char recvBuf[32];

	while (true) {
		bytesRecv = recv(s, recvBuf, 32, 0);
		cout << "Recieved " << bytesRecv << " bytes: " << recvBuf << '\n';

		bytesSent = send(s, sendBuf, 32, 0);
		cout << "Sent " << bytesRecv << " bytes: " << sendBuf << '\n';
	}
}
void Server::runListenThread() {
	while (!quit) {
		SOCKET newSocket = SOCKET_ERROR;

		while (newSocket == SOCKET_ERROR) {
			newSocket = accept(serverSocket, NULL, NULL);
		}

		cout << "Client connected\n";
		clientSockets.push_back(newSocket);
		threads.push_back(thread(&Server::handleClient, this, ref(clientSockets.end() - 1)));
	}
}
void Server::start() {
	if (!startListening(address, port)) {
		return;
	}

	threads.push_back(thread(&Server::runListenThread, this));

	while (!quit) {
		string comm;
		cin >> comm;
		if (comm == "quit") {
			quit = true;

			cout << "Closing sockets\n";
			closesocket(serverSocket);
			for (SOCKET& s : clientSockets) {
				closesocket(s);
			}
			cout << "Closed sockets\n";
		}

	}

	cout << "Waiting for threads to close themselves\n";
	for (auto& t : threads) {
		t.join();
	}
	cout << "Threads closed\n";
	threads.clear();
	clientSockets.clear();
	cout << "Server closed\n";
}