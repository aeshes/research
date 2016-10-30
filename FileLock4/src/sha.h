#ifndef SHA_H
#define SHA_H

#include <windows.h>
#include <wincrypt.h>

#define SHA1_HASH_LEN 20

BOOL SHA1PasswordHash(PBYTE lpszPassword, DWORD dwPasswordLen, PBYTE pbHash);

#endif
