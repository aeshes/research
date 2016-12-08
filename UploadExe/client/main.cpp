#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>
#include <stdint.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 666

void FormatError(DWORD error);
SOCKET RemoteConnect(char *ip, uint16_t port);
void SendFile(SOCKET sock, TCHAR *filename);

int main(int argc, char *argv[])
{
	WSADATA wsadata;
	SOCKET sock;

	if (FAILED(WSAStartup(MAKEWORD(2, 0), &wsadata)))
	{
		FormatError(WSAGetLastError());
	}
	else
	{
		sock = RemoteConnect("127.0.0.1", PORT);
		SendFile(sock, TEXT("calc.exe"));
		closesocket(sock);
	}

	WSACleanup();
}

void FormatError(DWORD errCode)
{
	char error[1000]; 
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
		error, sizeof(error), NULL); 
	printf("\nError: %s\n", error); 
	getchar();
}

SOCKET RemoteConnect(char *ip, uint16_t port)
{
	SOCKET sock;
	sockaddr_in remote_addr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		FormatError(WSAGetLastError());
	}
	else
	{
		RtlZeroMemory(&remote_addr, sizeof(remote_addr));
		remote_addr.sin_family = AF_INET;
		remote_addr.sin_port = htons(port);
		remote_addr.sin_addr.s_addr = inet_addr(ip);

		if (connect(sock, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) == SOCKET_ERROR)
		{
			FormatError(WSAGetLastError());
		}
	}

	return sock;
}

void SendFile(SOCKET sock, TCHAR *filename)
{
	HANDLE hFile = NULL;
	char buffer[200];
	DWORD bytes_read = 0;

	RtlZeroMemory(buffer, 200);
	hFile = CreateFile(filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("Error wile opening file\n");
		return;
	}
	else
	{
		while (TRUE)
		{
			ReadFile(hFile, buffer, 200, &bytes_read, NULL);
			int bytes_sent = send(sock, buffer, bytes_read, 0);
			if (bytes_sent > 0)
			{
				printf("%d bytes sent\n", bytes_sent);
			}
			else if (bytes_sent == 0)
			{
				printf("Connection closed\n");
				break;
			}
			else if (bytes_sent == SOCKET_ERROR)
			{
				FormatError(WSAGetLastError());
				break;
			}
		}

		CloseHandle(hFile);
	}
}