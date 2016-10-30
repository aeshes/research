#include "sha.h"

BOOL SHA1PasswordHash(PBYTE lpszPassword, DWORD dwPasswordLen, PBYTE pbHash)
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
			if (CryptHashData(hCryptHash, (LPBYTE)lpszPassword, dwPasswordLen, 0))
			{
				if (CryptGetHashParam(hCryptHash, HP_HASHVAL, bHashValue, &dwSize, 0))
				{
					for (i = 0, *pbHash = 0; i < SHA1_HASH_LEN; i++)
					{
						pbHash[i] = bHashValue[i];
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
