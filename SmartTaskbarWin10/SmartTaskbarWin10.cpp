// SmartTaskbarWin10.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"


APPBARDATA msgData;
WINDOWPLACEMENT placement;
HWND maxWindow;
POINT cursor;
DWORD uwpPID;
DWORD windowPID;
bool tryShowBar;

bool SetuwpPID()
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32First(snapshot, &entry) == TRUE)
		while (Process32Next(snapshot, &entry) == TRUE)
			if (_tcsicmp(entry.szExeFile, TEXT("ApplicationFrameHost.exe")) == 0) {
				uwpPID = entry.th32ProcessID;
				CloseHandle(snapshot);
				return true;
			}
	CloseHandle(snapshot);
	return false;
}

bool IsCursorOverTaskbar()
{
	GetCursorPos(&cursor);
	SHAppBarMessage(ABM_GETTASKBARPOS, &msgData);
	switch (msgData.uEdge)
	{
	case ABE_BOTTOM:
		if (cursor.y >= msgData.rc.top)
			return true;
		break;
	case ABE_LEFT:
		if (cursor.x <= msgData.rc.right)
			return true;
		break;
	case ABE_TOP:
		if (cursor.y <= msgData.rc.bottom)
			return true;
		break;
	default:
		if (cursor.x >= msgData.rc.left)
			return true;
		break;
	}
	return false;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	if (IsWindowVisible(hwnd) == FALSE)
		return TRUE;
	GetWindowPlacement(hwnd, &placement);
	if (placement.showCmd != SW_MAXIMIZE)
		return TRUE;
	GetWindowThreadProcessId(hwnd, &windowPID);
	if (uwpPID == windowPID)
		return TRUE;
	maxWindow = hwnd;
	return FALSE;
}

int main()
{
	tryShowBar = true;
	msgData.cbSize = sizeof(APPBARDATA);
	placement.length = sizeof(WINDOWPLACEMENT);
	while (true) {
		while (IsCursorOverTaskbar())
			Sleep(250);
		EnumWindows(EnumWindowsProc, NULL);
		if (maxWindow == NULL) {
			if (tryShowBar == false) {
				Sleep(375);
				continue;
			}
			tryShowBar = false;
			msgData.lParam = ABS_ALWAYSONTOP;
			SHAppBarMessage(ABM_SETSTATE, &msgData);
			Sleep(500);
			continue;
		}
		if (uwpPID == NULL)
			if (SetuwpPID()) {
				maxWindow = NULL;
				continue;
			}
		while (GetSystemMetrics(SM_CMONITORS) > 1)
			Sleep(1500);
		msgData.lParam = ABS_AUTOHIDE;
		SHAppBarMessage(ABM_SETSTATE, &msgData);
		do {
			Sleep(500);
			if (IsWindowVisible(maxWindow) == FALSE)
				break;
			GetWindowPlacement(maxWindow, &placement);
		} while (placement.showCmd == SW_MAXIMIZE);
		tryShowBar = true;
		maxWindow = NULL;
	}
	return 0;
}

