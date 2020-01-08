
#include "stdio.h";

#include "../wolf-bot-common/socketclient.h"
#include "Windows.h";
#include <time.h>
#include <tlhelp32.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "../wolf-bot-common/injector.h"


#pragma comment(lib, "opengl32")
#pragma comment(lib, "glu32")

void dejectPrevious()
{
	SocketClient socketClient = SocketClient();
	printf("attempting to deject\n");
	socketClient.clientConnect("127.0.0.1", "6666");
	Sleep(10);
	socketClient.clientSend("deject\r\n");
	Sleep(10);
	socketClient.clientClose();
	Sleep(10);
}

int main(int argc, char* argv[]) {
	dejectPrevious();

	if (!injectDll(argv[1])) {
		return 1;
	}

	printf("connecting to dll, input q to exit\n");

	SocketClient socketClient = SocketClient();
	if (socketClient.clientConnect("127.0.0.1", "6666"))
		printf("connected successfully\n");
	else {
		printf("connection failure\n");
		return 1;
	}

	char inputBuffer[256];

	while (scanf("%s", inputBuffer)) {
		if (strcmp(inputBuffer, "q") == 0) break;
		socketClient.clientSend(strcat(inputBuffer, "\r\n"));
	}

	socketClient.clientSend("deject\r\n");
	socketClient.clientClose();

	return 0;
}


