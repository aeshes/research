#include <windows.h>
#include <strsafe.h>
#include "crypt.h"

int main(int argc, char *argv[])
{
	CryptEngine crypt;
	char file[MAX_PATH]  = "C:\\files\\test\\1.txt";

	InitCryptEngine(&crypt);
	//FileEncrypt(file, &crypt);
	FileDecrypt(file, &crypt);

	FreeCryptEngine(&crypt);
}
