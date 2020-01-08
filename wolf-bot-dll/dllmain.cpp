#include "pch.h"

#include "../wolf-bot-common/socketserver.h"
#include <Windows.h>
#include <tgmath.h> 
#include <stdio.h>
#include <TlHelp32.h>	
#include <time.h>
#include <libloaderapi.h>
#include "detours.h"
#include <gl/gl.h>
#pragma comment (lib, "detours.lib")
#include "hack.h"
#include <stdlib.h>
#include "dllserver.h"

using namespace std;
HMODULE dllHModule;

ServerReceiveResult onReceiveData(const char* data) {
	if (strcmp(data, "deject") == 0) {
		socketServer.socketSend(loggingCallback, "dejecting!");
		return ServerReceiveResultEnd;
	}

	if (strcmp(data, "test") == 0) {
		socketServer.socketSend(loggingCallback, "test success!");
	}

	if (strncmp("bs-", data, 3) == 0) {
		char buffer[256];
		strcpy(buffer, data);
		char* p = strtok(buffer, "-");
		p = strtok(NULL, "-");
		if (p) {
			low_count = atoi(p);
		}
		p = strtok(NULL, "-");
		if (p) {
			high_count = atoi(p);
			socketServer.socketSend(loggingCallback, "set low, high %d %d", low_count, high_count);
		}
	}

	if (strcmp(data, "t-allies") == 0) {
		setTeam(TEAM_ALLIES);
	}
	if (strcmp(data, "t-axis") == 0) {
		setTeam(TEAM_AXIS);
	}

	return ServerReceiveResultContinue;
}

DWORD WINAPI runSocketServer(LPVOID) {
	socketServer.listenAndReceive("6666", loggingCallback, onReceiveData);
	removeHooks();
	Sleep(500);
	FreeLibraryAndExitThread(dllHModule, 0);
	return 0;
}

DWORD WINAPI initBot(LPVOID) {
	CreateThread(NULL, 0, runSocketServer, NULL, 0, NULL);
	Sleep(1000);
	setupHooks();
	return NULL;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	DisableThreadLibraryCalls(hModule);

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
		dllHModule = hModule;
		CreateThread(NULL, 0, initBot, NULL, 0, NULL);
		break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;

}
