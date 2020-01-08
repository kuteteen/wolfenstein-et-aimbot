#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H


#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdarg>
#include "pch.h"
#include <stdarg.h>

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512

typedef enum { ServerReceiveResultEnd, ServerReceiveResultContinue } ServerReceiveResult;

class SocketServer {
public:
	SOCKET ClientSocket = INVALID_SOCKET;

	bool socketSend(void (*loggingCallback)(const char* format, ...), const char* data, ...) {
		va_list args;
		va_start(args, data);
		char buffer[256];
		vsprintf(buffer, data, args);
		strcat(buffer, "\r\n");
		va_end(args);

		if (ClientSocket == INVALID_SOCKET)
			return false;

		int iSendResult = send(ClientSocket, buffer, strlen(buffer), 0);

		if (iSendResult == SOCKET_ERROR) {
			loggingCallback("send failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return false;
		}
		return true;
	}

	bool listenAndReceive(const char* port, void (*loggingCallback)(const char* format, ...), ServerReceiveResult(*receiveCallback)(const char*))
	{
		WSADATA wsaData;
		int iResult;

		SOCKET ListenSocket = INVALID_SOCKET;


		struct addrinfo* result = NULL;
		struct addrinfo hints;

		int iSendResult;
		char recvbuf[DEFAULT_BUFLEN];
		int recvbuflen = DEFAULT_BUFLEN;

		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			loggingCallback("WSAStartup failed with error: %d\n", iResult);
			return false;
		}

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		iResult = getaddrinfo(NULL, port, &hints, &result);
		if (iResult != 0) {
			loggingCallback("getaddrinfo failed with error: %d\n", iResult);
			WSACleanup();
			return false;
		}

		ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (ListenSocket == INVALID_SOCKET) {
			loggingCallback("socket failed with error: %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			return false;
		}

		iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			loggingCallback("bind failed with error: %d\n", WSAGetLastError());
			freeaddrinfo(result);
			closesocket(ListenSocket);
			WSACleanup();
			return false;
		}

		freeaddrinfo(result);

		iResult = listen(ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR) {
			loggingCallback("listen failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return false;
		}

		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			loggingCallback("accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return false;
		}

		closesocket(ListenSocket);

		do {

			iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {
				recvbuf[iResult - 2] = 0;
				if (receiveCallback(recvbuf) == ServerReceiveResultEnd) return false;
			}
			else if (iResult == 0) {
			}
			else {
				loggingCallback("recv failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return false;
			}

		} while (iResult > 0);

		iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			loggingCallback("shutdown failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return false;
		}

		closesocket(ClientSocket);
		WSACleanup();

		return true;
	}
};





#endif SOCKET_SERVER_H
