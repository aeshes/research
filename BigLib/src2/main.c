#include <windows.h>
#include <stdio.h>
#include "biglib.h"
#include "rsa.h"

#define RSA_BLOCK_SIZE	147

void encrypt(char *data)
{
	RSA_CONTEXT rsa;
	HANDLE hFile;
	DWORD dw;
	char block[RSA_BLOCK_SIZE];

	RSASetup(&rsa);
	ZeroMemory(block, RSA_BLOCK_SIZE);
	RSAEncrypt(&rsa, data, 16, block);
	RSAClear(&rsa);

	hFile = CreateFile(TEXT("rsa.txt"),
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	WriteFile(hFile, block, RSA_BLOCK_SIZE, &dw, NULL);
	CloseHandle(hFile);
}

void decrypt(char *data)
{
	RSA_CONTEXT rsa;
	HANDLE hFile;
	DWORD dw;
	char block[RSA_BLOCK_SIZE];

	ZeroMemory(block, RSA_BLOCK_SIZE);
	hFile = CreateFile(TEXT("rsa.txt"),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	ReadFile(hFile, block, RSA_BLOCK_SIZE, &dw, NULL);
	CloseHandle(hFile);

	RSASetup(&rsa);
	RSADecrypt(&rsa, block, RSA_BLOCK_SIZE, data);
	RSAClear(&rsa);
}

int main(int argc, char *argv[])
{
	char data[] = "abcdabcdabcdabcd";
	char decrypted[RSA_BLOCK_SIZE];

	encrypt(data);
	ZeroMemory(decrypted, RSA_BLOCK_SIZE);
	decrypt(decrypted);

	printf("decrypted = %s\n", decrypted);
}
