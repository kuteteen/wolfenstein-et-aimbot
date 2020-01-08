#pragma once


#include <Windows.h>
#include "detours.h"
#include <gl/gl.h>
#pragma comment (lib, "detours.lib")

#pragma comment(lib, "opengl32")
#include <regex>
#include <iostream>
#include "dllserver.h"
#include "models.h"
#include "constants.h"
#include "mainwindow.h"

using namespace std;

typedef void(WINAPI* _glDrawElements) (GLenum mode, GLsizei count, GLenum type, const GLvoid* indicies);
typedef void (WINAPI* _glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
typedef void(WINAPI* _glBindTexture) (GLenum target, GLuint texture);
typedef BOOL(WINAPI* _wglSwapBuffers) (_In_ HDC hDc);
typedef void(WINAPI* _glClear) (GLbitfield mask);

HMODULE opengl32 = GetModuleHandle("opengl32.dll");
_glDrawElements originalGlDrawElements = (_glDrawElements)GetProcAddress(opengl32, "glDrawElements");;
_glBindTexture originalGlBindTexture = (_glBindTexture)GetProcAddress(opengl32, "glBindTexture");
_wglSwapBuffers originalGlSwapBuffers = (_wglSwapBuffers)GetProcAddress(opengl32, "wglSwapBuffers");;
_glClear originalGlClear = (_glClear)GetProcAddress(opengl32, "glClear");;
_glReadPixels originalGlReadPixels = (_glReadPixels)GetProcAddress(opengl32, "glReadPixels");


char* renderedTexture = NULL;
BotCoordinate closestEnemyPixelToCrosshair = BotCoordinate(-1, -1);

#define isSnipingKeyDown() (GetKeyState(VK_XBUTTON1) & 0x8000)
#define isWallhackKeyDown() (GetKeyState(VK_XBUTTON2) & 0x8000)
#define isShootingKeyDown() (GetKeyState(VK_LBUTTON) & 0x8000)

typedef int TEAM;
#define TEAM_AXIS 0
#define TEAM_ALLIES 1

// used to perform a manual binary search when finding models by stride count in hookedGlDrawElements
int high_count = 0;
int low_count = 0;

char enemyTextureString[256];
int headshotLen = 0;

BotCoordinate screenCenter() {
	return BotCoordinate(X_RESOLUTION / 2, Y_RESOLUTION / 2);
}

void setTeam(TEAM team) {
	if (team == TEAM_AXIS) {
		logFromBot("setTeam axis");
		strcpy(enemyTextureString, "models/players/hud/allied");
	}
	else {
		logFromBot("setTeam allies");
		strcpy(enemyTextureString, "models/players/hud/axis");
	}
	headshotLen = strlen(enemyTextureString);
}

void click() {
	INPUT input;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	input.type = INPUT_MOUSE;
	input.mi.dy = 0;
	input.mi.dx = 0;
	input.mi.mouseData = 0;
	input.mi.dwExtraInfo = 0;
	input.mi.time = 0;
	SendInput(1, &input, sizeof(input));
	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &input, sizeof(input));
}

DWORD WINAPI reverseSnipingRecoil(LPVOID) {
	Sleep(500);
	INPUT input;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;
	input.type = INPUT_MOUSE;
	input.mi.dy = 260;
	input.mi.dx = 0;
	input.mi.mouseData = 0;
	input.mi.dwExtraInfo = 0;
	input.mi.time = 0;

	SendInput(1, &input, sizeof(input));
	return 0;
}

void applyAimBotMouseMovement(BotCoordinate shotCoordinate) {
	BotCoordinate diff = shotCoordinate.diff(screenCenter());
	INPUT input;
	double factor = 0.5;
	double maximum = 8.0;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;
	input.type = INPUT_MOUSE;

	if (abs(diff.x) < 3 && abs(diff.y < 3)) {
		factor = 1.0;
	}

	if (isSnipingKeyDown()) {
		factor = 1.0;
	}

	input.mi.dy = (long)(min((double)diff.y * factor, maximum));
	input.mi.dx = (long)(min((double)diff.x * factor, maximum));
	input.mi.mouseData = 0;
	input.mi.dwExtraInfo = 0;
	input.mi.time = 0;

	SendInput(1, &input, sizeof(input));

	if (isSnipingKeyDown() && diff.lessThan(2)) {
		click();
		CreateThread(NULL, 0, reverseSnipingRecoil, NULL, 0, NULL);
	}
	
}

Rgb pixels[10000000];
void readPixelsAndShoot() {
	closestEnemyPixelToCrosshair = BotCoordinate(10, 10);

	if ((!isShootingKeyDown() && !isSnipingKeyDown()) ||
		GetForegroundWindow() != mainWindow) {
		return;
	}

	BotCoordinate center = screenCenter();
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	originalGlReadPixels(center.x - STICKY_AIM_SIZE, center.y - STICKY_AIM_SIZE, STICKY_AIM_SIZE * 2, STICKY_AIM_SIZE * 2, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	// optimised by reading out from pixels at the center of the screen first. 
	// bottomLeftCorner begins in the center of the screen and moves to the bottom left corner of the sticky aim square. 
	// When a pixel is found matching the enemy colour, apply aimbot mouse movement
	long bottomLeftCorner = STICKY_AIM_SIZE - 1;
	long width = 2;
	while (bottomLeftCorner >= 0) {
		for (long y = 0; y < width; y++) {
			for (long x = 0; x < width; x++) {
				
				long ycoord = (bottomLeftCorner + y);
				long xcoord = bottomLeftCorner + x;
			
				Rgb p = pixels[xcoord + ycoord * (STICKY_AIM_SIZE + STICKY_AIM_SIZE)];
				
				if (p.rgbEqualPlayer()) {
					// y goes the other way when converting glReadPixel coordinates to screen coordinates
					closestEnemyPixelToCrosshair = BotCoordinate((X_RESOLUTION / 2 - STICKY_AIM_SIZE) + xcoord, (Y_RESOLUTION / 2 + STICKY_AIM_SIZE) - ycoord);
					applyAimBotMouseMovement(closestEnemyPixelToCrosshair);
					return;
				}
				// skip pixels we have already tested since we read inner pixels before the outer pixels
				if (y != 0 && y != width - 1 && x == 0) {
					x = width - 2;
				}
			}
		}
		bottomLeftCorner--; 
		width += 2;
	}
	closestEnemyPixelToCrosshair = BotCoordinate(10, 10);
}


void drawSquare(BotCoordinate center, long size) {
	glLineWidth(2.0);

	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 0.0f);

	glVertex2f(center.x - size, center.y - size);
	glVertex2f(center.x + size, center.y - size);

	glVertex2f(center.x - size, center.y + size);
	glVertex2f(center.x + size, center.y + size);

	glVertex2f(center.x - size, center.y - size);
	glVertex2f(center.x - size, center.y + size);

	glVertex2f(center.x + size, center.y - size);
	glVertex2f(center.x + size, center.y + size);

	glEnd();
}

void drawAimbotSquare() {
	drawSquare(screenCenter(), STICKY_AIM_SIZE + 3);
}

void drawAimAtDisplay() {
	if (closestEnemyPixelToCrosshair.x == -1) return;
	drawSquare(closestEnemyPixelToCrosshair, 5);
}


void WINAPI hookedGlDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indicies) {
	// Do not render center crosshair dot or sniping crosshair if sniping key down, because this interfers with aimbot
	if (renderedTexture && (strcmp(renderedTexture, "gfx/2d/crosshairp.tga") == 0 || (strcmp("*white", renderedTexture) == 0 && isSnipingKeyDown()))) {
		return;
	}

	// chams and wallhack implemented here
	if (renderedTexture && strncmp(enemyTextureString, renderedTexture, headshotLen) == 0 || count >= low_count && count <= high_count) {
		glDisable(GL_TEXTURE_2D);
		glDisableClientState(GL_COLOR_ARRAY);
		glEnable(GL_COLOR_MATERIAL);

		if (isWallhackKeyDown()) {
			glDepthFunc(GL_ALWAYS);
		}
		else {
			glDepthFunc(GL_LEQUAL);
		}
		// render enemies with green heads for aimbot to shoot at
		glColor3f(0.0f, 1.0f, 0.0f);
		originalGlDrawElements(mode, count, type, indicies);

		glDisable(GL_COLOR_MATERIAL);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnable(GL_TEXTURE_2D);

		glDepthFunc(GL_LEQUAL);
	}
	else {
		glDepthFunc(GL_LEQUAL);
		originalGlDrawElements(mode, count, type, indicies);
	}

}


BOOL WINAPI hookedGlSwapBuffers(_In_ HDC hDc)
{
	drawAimbotSquare();
	drawAimAtDisplay();
	return originalGlSwapBuffers(hDc);
}

void WINAPI hookedGlClear(GLbitfield mask)
{
	if (mask == GL_DEPTH_BUFFER_BIT)
	{
		mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
	}
	readPixelsAndShoot();

	(*originalGlClear)(mask);
}

void WINAPI hookedGlBindTexture(GLenum target, GLuint texture)
{
	// The texture string is consistently stored in esi register when glBindTexture gets called, which is useful :)
	__asm {
		mov renderedTexture, esi
	}
	(*originalGlBindTexture)(target, texture);
}


void setupHooks() {
	setTeam(TEAM_ALLIES);
	updateMainWindow();
	Sleep(50);

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)originalGlBindTexture, hookedGlBindTexture);
	DetourAttach(&(PVOID&)originalGlSwapBuffers, hookedGlSwapBuffers);
	DetourAttach(&(PVOID&)originalGlClear, hookedGlClear);
	DetourAttach(&(PVOID&)originalGlDrawElements, hookedGlDrawElements);
	DetourTransactionCommit();
	
}

void removeHooks() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)originalGlDrawElements, hookedGlDrawElements);
	DetourDetach(&(PVOID&)originalGlClear, hookedGlClear);
	DetourDetach(&(PVOID&)originalGlBindTexture, hookedGlBindTexture);
	DetourDetach(&(PVOID&)originalGlSwapBuffers, hookedGlSwapBuffers);
	DetourTransactionCommit();
}
