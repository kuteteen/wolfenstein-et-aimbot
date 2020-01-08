#pragma once
#include "stdio.h";

#include "Windows.h";
#include <time.h>
#include <tlhelp32.h>



bool injectDll(const char* dllPath) {

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	printf("Injecting...\n");

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	HANDLE process = NULL;
	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (_stricmp(entry.szExeFile, "etl.exe") == 0)
			{
				process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
			}
		}
	}

	CloseHandle(snapshot);

	if (process == NULL) {
		printf("Error: the specified process couldn't be found.\n");
		return false;
	}

	LPVOID addr = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
	if (addr == NULL) {
		printf("Error: the LoadLibraryA function was not found inside kernel32.dll library.\n");

		return false;
	}

	LPVOID arg = (LPVOID)VirtualAllocEx(process, NULL, strlen(dllPath), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (arg == NULL) {
		printf("Error: the memory could not be allocated inside the chosen process.\n");
		return false;
	}

	int n = WriteProcessMemory(process, arg, dllPath, strlen(dllPath), NULL);
	if (n == 0) {
		printf("Error: there was no bytes written to the process's address space.\n");
		return false;
	}

	HANDLE threadID = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)addr, arg, NULL, NULL);
	if (threadID == NULL) {
		printf("Error: the remote thread could not be created.\n");
		return false;
	}
	else {
		printf("dll injection successful\n");
	}

	CloseHandle(process);
	return true;
}
