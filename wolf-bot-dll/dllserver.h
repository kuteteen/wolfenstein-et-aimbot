#include "../wolf-bot-common/socketserver.h"
#include <stdarg.h>

#ifndef DLLSERVER_H
#define DLLSERVER_H
SocketServer socketServer = SocketServer();

void loggingCallback(const char* format, ...) {
	// write to filesystem to debug
}

void logFromBot(const char* format, ...) {
	va_list args;
	va_start(args, format);
	socketServer.socketSend(loggingCallback, format, args);
	va_end(args);
}

#endif DLLSERVER_H
