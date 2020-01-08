#include "pch.h"
#include <cstdarg>
#include <string>
#include "stdio.h"
#include <windows.h>
#include <stdio.h>
#include <time.h>


void timestamp(char* output)
{
	time_t timer;
	struct tm* tm_info;

	time(&timer);
	tm_info = localtime(&timer);

	SYSTEMTIME time;
	GetSystemTime(&time);
	char buffer[256];
	strftime(buffer, 26, "%Y-%m-%d %H:%M:%S:%%d - ", tm_info);

	sprintf(output, buffer, time.wMilliseconds);
}

__int64 epoch() {
	SYSTEMTIME time;
	GetSystemTime(&time);
	return (__int64)time.wMilliseconds + (__int64)time.wSecond * 1000ll + (__int64)time.wMinute * 60ll * 1000ll + (__int64)time.wHour * 60ll * 60ll * 10000ll + (long)time.wDay * 24ll * 60ll * 60ll * 1000ll;
}


void logToFile(const char * filename, const char* format, ...) {
	FILE* file = _fsopen(filename, "a+", _SH_DENYNO);
	va_list args;
	va_start(args, format);
	char buffer[256];
	timestamp(buffer);
	strcpy(buffer + strlen(buffer), format);
	vfprintf(file, buffer, args);
	va_end(args);
	fclose(file);
}