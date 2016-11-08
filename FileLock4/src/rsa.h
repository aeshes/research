#ifndef RSA_H
#define RSA_H

#include "biglib.h"
#include "crypt.h"

#define RSA_BLOCK_SIZE	147

void RSAEncrypt(CryptEngine *engine, char *src, DWORD len, char *dst);
void RSADecrypt(CryptEngine *engine, char *src, DWORD len, char *dst);

#endif
