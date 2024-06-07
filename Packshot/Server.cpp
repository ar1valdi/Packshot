#include "Server.h"
#include <WS2tcpip.h>
using namespace std;

Server::Server(const string& address, int port) {
	this->address = address;
	this->port = port;
}
bool Server::startListening(const string& address, int port) {
	WSADATA wsadata;

	int err = WSAStartup((2, 2), &wsadata);
	if (err != NO_ERROR) {
		cout << "WSA init error: " << WSAGetLastError();
		return false;
	}

	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET) {
		cout << "Error creating client socket: " << WSAGetLastError();
		WSACleanup();
		return false;
	}

	sockaddr_in service;
	memset(&service, 0, sizeof(service));
	service.sin_family = AF_INET;
	InetPton(AF_INET, __TEXT("127.0.0.1"), &service.sin_addr.s_addr);
	service.sin_port = htons(port);

	if (bind(serverSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
		cout << "Error while binding: " << WSAGetLastError();
		closesocket(serverSocket);
		WSACleanup();
		return false;
	}

	err = listen(serverSocket, 1);
	if (err == SOCKET_ERROR) {
		cout << "Failed to start listening: " << WSAGetLastError();
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

	u_long mode = 1;
	ioctlsocket(s, FIONBIO, &mode);

	while (!quit) {
		int err = WSAEWOULDBLOCK;
		while (err == WSAEWOULDBLOCK) {
			bytesRecv = recv(s, recvBuf, 32, 0);
			this_thread::sleep_for(chrono::milliseconds(100));
			err = WSAGetLastError();
		}

		if (bytesRecv == 0) {
			cout << "Connection closed\n";
			break;
		}
		else if (bytesRecv < 0) {
			cout << "recv failed: " << err << '\n';
			break;
		}
		cout << "Recieved " << bytesRecv << " bytes: " << recvBuf << '\n';

		// tutaj bd wywolanie metody game

		err = WSAEWOULDBLOCK;
		while (err == WSAEWOULDBLOCK) {
			bytesSent = send(s, recvBuf, 32, 0);
			this_thread::sleep_for(chrono::milliseconds(100));
			err = WSAGetLastError();
		}

		if (bytesSent == 0) {
			cout << "Connection closed\n";
			break;
		}
		else if (bytesSent < 0) {
			perror("send failed: ");
			break;
		}
		cout << "Sent " << bytesSent << " bytes: " << sendBuf << '\n';
	}

	closesocket(s);
	//clientSockets.erase(find(clientSockets.begin(), clientSockets.end(), s));
	cout << "Closing thread " << this_thread::get_id();
}
void Server::runListenThread() {
	while (!quit) {
		SOCKET newSocket = SOCKET_ERROR;

		while (newSocket == SOCKET_ERROR && !quit) {
			newSocket = accept(serverSocket, NULL, NULL);
		}

		if (quit) { 
			break;
		}

		cout << "Client connected\n";
		clientSockets.push_back(newSocket);
		threads.push_back(thread(&Server::handleClient, this, ref(*(clientSockets.end() - 1))));
	}
}
void Server::start() {
	if (!startListening(address, port)) {
		return;
	}

	threads.push_back(thread(&Server::runListenThread, this));
	cout << "Enter 'quit' to shutdown server\n";
	while (!quit) {
		string comm;
		cin >> comm;
		if (comm == "quit") {
			quit = true;
		}

	}

	closesocket(serverSocket);
	cout << "Waiting for threads to close themselves\n";
	for (auto& t : threads) {
		t.join();
	}
	cout << "Threads closed\n";
	threads.clear();
	clientSockets.clear();
	WSACleanup();
	cout << "Server closed\n";
}