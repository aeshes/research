#include <windows.h>
#include "xtea.h"

void encrypt(char *filename, void *xtea)
{
	HANDLE file;
	DWORD file_size;
	DWORD bytes_read = 0, bytes_written = 0;
	int i;
	char buffer[XTEA_BLOCK_SIZE_BYTES + 1] = { 0 };

	file = CreateFile(filename,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (file == INVALID_HANDLE_VALUE) return;

	file_size = GetFileSize(file, NULL);
	if (file_size == INVALID_FILE_SIZE) return;
	if (file_size < XTEA_BLOCK_SIZE_BYTES * BLOCKS_TO_ENCRYPT) return;

	// Encrypt file header
	for (i = 0; i < BLOCKS_TO_ENCRYPT; i++)
	{
		ZeroMemory(buffer, XTEA_BLOCK_SIZE_BYTES + 1);
		SetFilePointer(file, i * XTEA_BLOCK_SIZE_BYTES, NULL, FILE_BEGIN);
		ReadFile(file, buffer, XTEA_BLOCK_SIZE_BYTES, &bytes_read, NULL);
		xtea_encrypt(64, (uint32_t *)buffer, (uint32_t *)((XTEA_DATA *)xtea)->xtea_key);
		SetFilePointer(file, i * XTEA_BLOCK_SIZE_BYTES, NULL, FILE_BEGIN);
		WriteFile(file, buffer, XTEA_BLOCK_SIZE_BYTES, &bytes_written, NULL);
	}

	CloseHandle(file);
}

void decrypt(char *filename, void *xtea)
{
	HANDLE file;
	DWORD file_size;
	DWORD bytes_read = 0, bytes_written = 0;
	int i;
	char buffer[XTEA_BLOCK_SIZE_BYTES + 1] = { 0 };

	file = CreateFile(filename,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (file == INVALID_HANDLE_VALUE) return;

	file_size = GetFileSize(file, NULL);
	if (file_size == INVALID_FILE_SIZE) return;

	// Decryption
	for (i = 0; i < BLOCKS_TO_ENCRYPT; i++)
	{
		ZeroMemory(buffer, XTEA_BLOCK_SIZE_BYTES);
		SetFilePointer(file, i * XTEA_BLOCK_SIZE_BYTES, NULL, FILE_BEGIN);
		ReadFile(file, buffer, XTEA_BLOCK_SIZE_BYTES, &bytes_read, NULL);
		xtea_decrypt(64, (uint32_t *)buffer, (uint32_t *)((XTEA_DATA *)xtea)->xtea_key);
		SetFilePointer(file, i * XTEA_BLOCK_SIZE_BYTES, NULL, FILE_BEGIN);
		WriteFile(file, buffer, XTEA_BLOCK_SIZE_BYTES, &bytes_written, NULL);
	}

	CloseHandle(file);
}

int main()
{
	XTEA_DATA xtea;
	char file[MAX_PATH] = "C:\\files\\test\\1.jpg";
	char temp[MAX_PATH] = "C:\\files\\test\\1.jpg";
	char text[MAX_PATH] = "C:\\files\\test\\1.txt";

	xtea.KEY_SIZE = 16;
	xtea.XTEA_KEY_RANGE = 58;
	CopyMemory(xtea.xtea_key, "abcdabcdabcdabcd", 16);

	encrypt(file, &xtea);
	decrypt(temp, &xtea);
}