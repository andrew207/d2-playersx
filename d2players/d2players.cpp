#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <sstream>
#include <tlhelp32.h>

using namespace std;

HANDLE OpenProcessByName(LPCTSTR Name, DWORD dwAccess)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe;
		ZeroMemory(&pe, sizeof(PROCESSENTRY32));
		pe.dwSize = sizeof(PROCESSENTRY32);
		Process32First(hSnap, &pe);
		do
		{
			if (!lstrcmpi(pe.szExeFile, Name))
			{
				return OpenProcess(dwAccess, 0, pe.th32ProcessID);
			}
		} while (Process32Next(hSnap, &pe));

	}
	return INVALID_HANDLE_VALUE;
}

/*
Memory Location of Players var
<RealAddress>"0087BDB0" </RealAddress>
<Address>Game.exe+47BDB0</Address>
*/

int main(int argc, char* argv[])
{
	DWORD address = 0x0087BDB0; // mem addy for playersx
	int value = 0;

	// Get SeDebugPrivilege
	TOKEN_PRIVILEGES NewState;
	NewState.PrivilegeCount = 1;
	if (!LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &NewState.Privileges[0].Luid)) {
		std::clog << "Could not acquire debug-privilege name: " << GetLastError() << "\n";
		return EXIT_FAILURE;
	}
	HANDLE token;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token)) {
		std::clog << "Could not acquire process token: " << GetLastError() << "\n";
		return EXIT_FAILURE;
	}
	NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(token, FALSE, &NewState, sizeof(NewState), nullptr, nullptr)) {
		std::clog << "Could not enable debug privilege: " << GetLastError() << "\n";
		return EXIT_FAILURE;
	}
	std::clog << "Acquired debug privilege\n";

	// Make sure we actually opened something
	HANDLE hwnd = OpenProcessByName("Game.exe", PROCESS_VM_READ);

	
	if (hwnd && hwnd != INVALID_HANDLE_VALUE) {
		cout << "Game.exe Handle found: 0x" << hwnd << endl;
	}
	else {
		cout << "\rCould not fine game.exe handle, are you running D2 Game.exe? Exiting in 5s.";
		Sleep(5000);
		return 0;
	}

	while (1) {
		ReadProcessMemory(hwnd, (void*)address, &value, sizeof(value), 0);
		cout << "\rPlayers: " << value;
		Sleep(1000);
	}

	return 0;
}