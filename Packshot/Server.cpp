#include "Server.h"
#include "Consts.h"
#include <WS2tcpip.h>
#include <string>
#include <locale>
#include <codecvt>
using namespace std;

Server::Server(const string& address, int port) {
	this->address = address;
	this->port = port;
	this->game = new Game();
	for (auto& p : threads) {
		p.second = false;
	}
	this->connectedClients = 0;
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
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	wstring wip = converter.from_bytes(address);
	const wchar_t* Lip = wip.c_str();
	InetPton(AF_INET, Lip, &service.sin_addr.s_addr);
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
	char recvBuf[SERVER_RECV_BUF];

	u_long mode = 1;
	ioctlsocket(s, FIONBIO, &mode);

	while (!quit) {
		int err = WSAEWOULDBLOCK;
		while (err == WSAEWOULDBLOCK && !quit) {
			bytesRecv = recv(s, recvBuf, 1024, 0);
			this_thread::sleep_for(chrono::milliseconds(100));
			err = WSAGetLastError();
		}
		if (quit) {
			break;
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

		string sendBuf;
		if (strcmp(recvBuf, FETCH_MSG) == 0) {
			sendBuf = game->getSerializedGameState();
		}
		else {
			Action a = Action::deserialize(recvBuf);
			GameState gs = game->handleRequest(a);
			sendBuf = gs.serialize();
		}

		bytesSent = send(s, sendBuf.c_str(), sendBuf.length() + 1, 0);

		if (bytesSent == 0) {
			cout << "Connection closed\n";
			break;
		}
		else if (bytesSent < 0) {
			cout << "send failed: " << WSAGetLastError();
			break;
		}
		cout << "Sent " << bytesSent << " bytes: " << sendBuf << '\n';
	}

	cleanClientThreadData(s, this_thread::get_id());
	cout << "Closing thread " << this_thread::get_id();
}
void Server::cleanClientThreadData(SOCKET& s, thread::id threadID) {
	closesocket(s);

	lock_guard<mutex> lock(mtx);
	clientSockets.erase(find(clientSockets.begin(), clientSockets.end(), s));

	for (int i = 0; i < MAX_CLIENTS_NUM; i++) {
		if (threads[i].first.get_id() == threadID) {
			threads[i].second = false;
			break;
		}
	}
}
void Server::addThread(SOCKET& s) {
	mtx.lock();
	clientSockets.push_back(s);
	for (int i = 1; i < MAX_CLIENTS_NUM; i++) {
		if (threads[i].second == false) {
			if (threads[i].first.joinable()) {
				threads[i].first.join();
			}
			threads[i].first = thread(&Server::handleClient, this, ref(*(clientSockets.end() - 1)));
			threads[i].second = true;
			break;
		}
		if (i == MAX_CLIENTS_NUM) {
			throw runtime_error("can't find thread for connection");
		}
	}
	mtx.unlock();
}
void Server::runListenThread() {
	while (!quit) {
		SOCKET newSocket = SOCKET_ERROR;

		while ((newSocket == SOCKET_ERROR && !quit) || connectedClients >= MAX_CLIENTS_NUM) {
			newSocket = accept(serverSocket, NULL, NULL);
		}

		if (quit) { 
			break;
		}

		cout << "Client connected\n";

		addThread(newSocket);
	}
}
void Server::start() {
	game->start();

	if (!startListening(address, port)) {
		return;
	}

	threads[0].first = thread(&Server::runListenThread, this);
	threads[0].second = true;
	cout << "Enter 'quit' to shutdown server\n";
	while (!quit) {
		string comm;
		cin >> comm;
		if (comm == "quit") {
			quit = true;
		}

	}

	game->stop();

	closesocket(serverSocket);
	cout << "Waiting for threads to close themselves\n";
	for (auto& t : threads) {
		if (t.first.joinable()) {
			t.first.join();
		}
	}
	cout << "Threads closed\n";
	clientSockets.clear();
	WSACleanup();
	cout << "Server closed\n";
}
Server::~Server() {
	delete game;
}