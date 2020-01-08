#pragma once

#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512


class SocketClient {
public:
	SOCKET ConnectSocket = INVALID_SOCKET;
	bool clientConnect(const char* address, const char* port) {
		WSADATA wsaData;
		struct addrinfo* result = NULL,
			* ptr = NULL,
			hints;
		char recvbuf[DEFAULT_BUFLEN];
		int iResult;
		int recvbuflen = DEFAULT_BUFLEN;


		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			printf("WSAStartup failed with error: %d\n", iResult);
			return false;
		}

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		iResult = getaddrinfo(address, port, &hints, &result);
		if (iResult != 0) {
			printf("getaddrinfo failed with error: %d\n", iResult);
			WSACleanup();
			return false;
		}

		for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

			ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
				ptr->ai_protocol);
			if (ConnectSocket == INVALID_SOCKET) {
				printf("socket failed with error: %ld\n", WSAGetLastError());
				WSACleanup();
				return false;
			}

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
			return false;
		}

		CreateThread(NULL, 0, printReceived, this, 0, NULL);

		return true;
	}


	bool clientSend(const char* data)
	{
		if (ConnectSocket == INVALID_SOCKET) return 1;

		struct addrinfo* result = NULL,
			*ptr = NULL,
			hints;
		char recvbuf[DEFAULT_BUFLEN];
		int iResult;
		int recvbuflen = DEFAULT_BUFLEN;


		iResult = send(ConnectSocket, data, (int)strlen(data), 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return false;
		}

		return true;
	}

	void clientClose() {
		if (ConnectSocket == INVALID_SOCKET) return;

		int iResult = shutdown(ConnectSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			printf("shutdown failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
		}

		closesocket(ConnectSocket);
		WSACleanup();

		ConnectSocket = INVALID_SOCKET;
	}

	static DWORD WINAPI printReceived(LPVOID lparam) {
		SocketClient* client = (SocketClient*)lparam;
		char recvbuf[256];
		int iResult;
		while ((iResult = recv(client->ConnectSocket, recvbuf, 256, 0)) > 0) {
			recvbuf[iResult - 2] = 0;
			printf("socket server says: %s\n", recvbuf);
		}
	}
};




#endif SOCKET_CLIENT_H
