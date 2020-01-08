#include <Windows.h>

HWND mainWindow = NULL;
HDC deviceContext = NULL;

BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam)
{
	DWORD lpdwProcessId;
	GetWindowThreadProcessId(hwnd, &lpdwProcessId);
	if (lpdwProcessId == lParam)
	{
		mainWindow = hwnd;
		deviceContext = GetDC(mainWindow);
		return FALSE;
	}
	return TRUE;
}

void updateMainWindow() {
	EnumWindows(enumWindowsProc, GetCurrentProcessId());
}