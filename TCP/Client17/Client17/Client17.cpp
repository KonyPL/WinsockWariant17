#include "pch.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <string>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"


int __cdecl main(int argc, char** argv)
{

	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	// Validate the parameters
	/*if (argc != 2) {
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}*/

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	char ADDR[16], PORT[6];
	std::string addr, port;
	//Get Server address and Port
	do {
		std::cout << "Type the server address (default 127.0.0.1 - press Enter): ";
		std::getline(std::cin, addr);
		if (addr == "") addr = "127.0.0.1";

		addr.copy(ADDR, (addr.size() + 1));
		ADDR[addr.size()] = 0;

		std::cout << "Type the server port (default: 27015 - press Enter): ";
		std::getline(std::cin, port);
		if (port == "") port = DEFAULT_PORT;

		port.copy(PORT, (port.size() + 1));
		PORT[port.size()] = 0;

		// Resolve the server address and port
		iResult = getaddrinfo(ADDR, PORT, &hints, &result);
		if (iResult != 0) {
			printf("getaddrinfo failed with error: %d\n", iResult);
			//WSACleanup();
			//return 1;
		}
	} while (iResult != 0);

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	// Random value that is going to be send to server
	std::srand(std::time(NULL));
	int random_variable = std::rand() % 10001;
	char sendBuff[4] = { 0 };

	std::memcpy(sendBuff, &random_variable, sizeof random_variable); // Changes integer value into byte array

	// Send an initial buffer
	iResult = send(ConnectSocket, (char*)sendBuff, sizeof(sendBuff), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	printf("Random number sent: %ld\n", random_variable);


	// Shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}


	char recvBuff[4] = { 0 };
	int re = 0;

	// Receive until the peer closes the connection
	do {

		iResult = recv(ConnectSocket, recvBuff, sizeof(recvBuff), 0);
		if (iResult > 0)
		{
			std::memcpy(&re, recvBuff, sizeof recvBuff); // Changes byte array into integer value
			printf("Prime number received: %d\n", re);
		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());

	} while (iResult > 0);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}