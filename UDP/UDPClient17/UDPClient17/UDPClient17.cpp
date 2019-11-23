#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctime>
#include <cstdlib>
#include <iostream>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int recvTimeOutUDP(SOCKET socket, long sec, long usec)
{
	// Setup timeval variable
	struct timeval timeout;
	struct fd_set fds;

	timeout.tv_sec = sec;
	timeout.tv_usec = usec;
	// Setup fd_set structure
	FD_ZERO(&fds);
	FD_SET(socket, &fds);
	// Return value:
	// -1: error occurred
	// 0: timed out
	// > 0: data ready to be read
	return select(0, &fds, 0, 0, &timeout);
}

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

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	// Resolve the server address and port
	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

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
	iResult = sendto(ConnectSocket, (char*)sendBuff, sizeof(sendBuff), 0, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	printf("Random number sent: %ld\n", random_variable);

	char recvBuff[4] = { 0 };
	int re = 0;

	do {

		int SelectTiming = recvTimeOutUDP(ConnectSocket, 1, 0);

		switch (SelectTiming)
		{
		case 0:
			// Timed out
			iResult = sendto(ConnectSocket, (char*)sendBuff, 0, 0, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			printf("Connection closing...");
			iResult = 0;
			break;
		case -1:
			// Error occurred
			printf("An error occured while waiting for timeout : % ld\n", WSAGetLastError());
			break;
		default:
			// Ok the data is ready
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
		}

	} while (iResult > 0);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}