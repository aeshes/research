#ifndef RSA_H
#define RSA_H

#include "biglib.h"

typedef struct RSA_CONTEXT
{
	DWORD modulus;
	DWORD encryption_key;
	DWORD decryption_key;
} RSA_CONTEXT;

void RSASetup(RSA_CONTEXT *);
void RSAClear(RSA_CONTEXT *);
void RSAEncrypt(RSA_CONTEXT *, char *src, DWORD len, char *dst);
void RSADecrypt(RSA_CONTEXT *, char *src, DWORD len, char *dst);

#endif
