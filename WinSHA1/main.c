#include <windows.h>
#include <wincrypt.h>

#define SHA1_HASH_LEN 20

BOOL SHA1PasswordHash(LPCTSTR lpszPassword, LPTSTR lpszHash)
{
	HCRYPTPROV hCryptProv;
	HCRYPTHASH hCryptHash;
	BYTE	   bHashValue[SHA1_HASH_LEN];
	DWORD	   dwSize = SHA1_HASH_LEN;
	BOOL	   bSuccess = FALSE;
	int i;

	if (CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		if(CryptCreateHash(hCryptProv, CALG_SHA1, 0, 0, &hCryptHash))
		{
			if (CryptHashData(hCryptHash, (LPBYTE)lpszPassword, lstrlen(lpszPassword) * sizeof(TCHAR), 0))
			{
				if (CryptGetHashParam(hCryptHash, HP_HASHVAL, bHashValue, &dwSize, 0))
				{
					for (i = 0, *lpszHash = 0; i < SHA1_HASH_LEN; i++)
					{
						wsprintf(lpszHash + i * 2, TEXT("%02x"), bHashValue[i]);
					}
					bSuccess = TRUE;
				}
			}
			CryptDestroyHash(hCryptHash);
		}
		CryptReleaseContext(hCryptProv, 0);
	}
	return bSuccess;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow)
{
	LPCTSTR lpszPassword = TEXT("Hello!");
	LPTSTR  lpszHash	 = LocalAlloc(LPTR, SHA1_HASH_LEN * 2 + 1);

	if (lpszHash == NULL)
	{
		MessageBox(NULL, TEXT("Error: cant allocate memory"), TEXT("Error"), MB_OK | MB_ICONERROR);
	}

	if (SHA1PasswordHash(lpszPassword, lpszHash))
	{
		MessageBox(NULL, lpszHash, TEXT("Hash value"), MB_OK);
	}
	else MessageBox(NULL, TEXT("Error: cant create hash"), TEXT("Error"), MB_OK | MB_ICONERROR);

	LocalFree(lpszHash);

	return 0;
}