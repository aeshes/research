#include <windows.h>
#include <string.h>
#include <stdio.h>

#pragma comment(lib, "user32.lib")

void RunNewProcess(char *szExePath)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	HANDLE hCurrentProcess = NULL;
	char szCmdLine[MAX_PATH];

	DuplicateHandle(GetCurrentProcess(),
		GetCurrentProcess(),
		GetCurrentProcess(),
		&hCurrentProcess,
		0,
		TRUE,
		DUPLICATE_SAME_ACCESS);

	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	wsprintf(szCmdLine, "%s %d", szExePath, (int)hCurrentProcess);
	CreateProcess(NULL,
		szCmdLine,
		NULL,
		NULL,
		TRUE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&si,
		&pi);
}

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

int main(int argc, char *argv[])
{
	HANDLE hParentProcess = NULL;
	HANDLE hThread = NULL;
	char szExePath[MAX_PATH];

	GetModuleFileName(NULL, szExePath, MAX_PATH);

	if (argv[1] == NULL)	// First run
	{
		//RunNewProcess(szExePath);
	}
	else hParentProcess = (HANDLE) atoi(argv[1]);

	WaitForSingleObject(hParentProcess, INFINITE);
	RunNewProcess(szExePath);

	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread, NULL, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);
}
