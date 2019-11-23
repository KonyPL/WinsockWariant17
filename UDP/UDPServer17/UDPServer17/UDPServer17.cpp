#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <math.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"


int __cdecl main(void)
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	SOCKADDR_IN        SenderAddr;
	int                SenderAddrSize = sizeof(SenderAddr);

	struct addrinfo* result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
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
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("Receiving socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	ClientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (ClientSocket == INVALID_SOCKET) {
		printf("Sending socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the UDP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	// Receive until the peer shuts down the connection
	do {

		char sendBuff[4] = { 0 };
		char recvBuff[4] = { 0 };
		//iResult = recv(ClientSocket, recvBuff, sizeof(recvBuff), 0);
		iResult = recvfrom(ListenSocket, recvBuff, sizeof(recvBuff), 0, (SOCKADDR*)&SenderAddr, &SenderAddrSize);

		if (iResult > 0) {
			int re = 0;

			std::memcpy(&re, recvBuff, sizeof recvBuff); // Changes byte array into integer value

			printf("Number received: %d\n", re);

			// Send prime numbers lower than received value
			for (int i = 2; i < re; i++)
			{
				for (int j = 2; j * j <= i; j++)
				{
					if (i % j == 0)
					{
						break;
					}
					else if (j + 1 > sqrt(i)) {

						std::memcpy(sendBuff, &i, sizeof i); // Changes integer value into byte array

						iSendResult = sendto(ClientSocket, (char*)sendBuff, sizeof(sendBuff), 0, (SOCKADDR*)&SenderAddr, SenderAddrSize);

						if (iSendResult == SOCKET_ERROR) {
							printf("send failed with error: %d\n", WSAGetLastError());
							closesocket(ClientSocket);
							WSACleanup();
							return 1;
						}
						printf("Prime number sent: %d\n", i);
					}

				}
			}
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

	} while (iResult > 0);

	// cleanup
	closesocket(ListenSocket);
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}