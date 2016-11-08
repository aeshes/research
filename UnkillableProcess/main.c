#include <windows.h>
#include <string.h>
#include <stdio.h>

#pragma comment(lib, "user32.lib")


void thread(void)
{
	int i = 0;
	while (1)
	{
		++i;
		printf("%d ", i);
		Sleep(500);
	}
}

void TrackChildProcess(LPVOID param)
{
	while(1)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		CHAR szExePath[MAX_PATH];
		CHAR szCmdLine[MAX_PATH];
		DWORD pid = GetCurrentProcessId();

		GetModuleFileName(GetModuleHandle(NULL), szExePath, MAX_PATH);
		wsprintf(szCmdLine,"%s %d\0", szExePath, pid);

		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		CreateProcess(NULL,
			szCmdLine,
			NULL,
			NULL,
			FALSE,
			CREATE_NEW_CONSOLE,
			NULL,
			NULL,
			&si,
			&pi);

		WaitForSingleObject(pi.hProcess, INFINITE);
	}
}

void TrackParentProcess(LPVOID hParentProcess)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	CHAR szExePath[MAX_PATH];

	WaitForSingleObject((HANDLE)hParentProcess, INFINITE);
	
	GetModuleFileName(GetModuleHandle(NULL), szExePath, MAX_PATH);

	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	CreateProcess(NULL,
		szExePath,
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&si,
		&pi);
	ExitProcess(0);
}

int main(int argc, char *argv[])
{
	HANDLE hParentProcess = NULL;
	DWORD  ProcessID 	  = 0;

	if (argv[1] != NULL)
		ProcessID = atoi(argv[1]);

	if (ProcessID == 0)
	{
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TrackChildProcess, NULL, 0, NULL);
	}
	else
	{
		hParentProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);
		if (hParentProcess)
		{
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TrackParentProcess, (LPVOID)hParentProcess, 0, NULL);
			while (1)
			{
				Sleep(500);
			}
		}
	}

	// Main logic goes here...
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread, NULL, 0, NULL);
	while(1) Sleep(500);
}
