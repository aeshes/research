#include "crypt.h"
#include <windows.h>

void crypt(char *filename, void *xtea)
{
	HANDLE file, new_file;
	DWORD file_size;
	DWORD bytes_read = 0, bytes_written = 0;
	char buffer[9] = { 0 };;
	char *last_char = filename + lstrlen(filename);

	file = CreateFile(filename,
					  GENERIC_READ,
					  FILE_SHARE_READ,
					  NULL,
					  OPEN_EXISTING,
					  FILE_ATTRIBUTE_NORMAL,
					  NULL);
	if (file == INVALID_HANDLE_VALUE) return;

	file_size = GetFileSize(file, NULL);
	if (file_size == INVALID_FILE_SIZE) return;

	lstrcat(filename, ".tmp");
	new_file = CreateFile(filename,
						  GENERIC_READ | GENERIC_WRITE,
						  FILE_SHARE_READ,
						  NULL,
						  CREATE_ALWAYS,
						  FILE_ATTRIBUTE_NORMAL,
						  NULL);
	if (new_file == INVALID_HANDLE_VALUE) return;

	// Encryption
	// Write hash
	/*WriteFile(new_file,
			 &((XTEA_DATA *)xtea)->XTEA_KEY_HASH,
			 4,
			 &bytes_written,
			 NULL);
	// Write key
	WriteFile(new_file,
			  (LPVOID)((XTEA_DATA *)xtea)->xtea_key,
			  ((XTEA_DATA *)xtea)->KEY_SIZE,
			  &bytes_written,
			  NULL);*/
	// Crypt file data and write
	do
	{
		ZeroMemory(buffer, 9);
		ReadFile(file, buffer, 8, &bytes_read, NULL);
		xtea_decrypt(64, (uint32_t *)buffer, (uint32_t *)((XTEA_DATA *)xtea)->xtea_key);
		WriteFile(new_file, buffer, bytes_read, &bytes_written, NULL);
	} while (bytes_read != 0);

	CloseHandle(file);
	CloseHandle(new_file);
}