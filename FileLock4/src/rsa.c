#include "crypt.h"


void RSAEncrypt(CryptEngine *engine, BYTE *src, DWORD len, BYTE *dst)
{
	DWORD block = _BigCreate(0);
	DWORD encrypted = _BigCreate(0);

	_BigInB256(src, len, block);
	_BigPowMod(block, engine->RSA.pEncryptionKey, engine->RSA.pModulus, encrypted);
	_BigOutB256(encrypted, dst);

	_BigDestroy(block);
	_BigDestroy(encrypted);
}

void RSADecrypt(CryptEngine *engine, BYTE *src, DWORD len, BYTE *dst)
{
	DWORD block = _BigCreate(0);
	DWORD decrypted = _BigCreate(0);

	_BigInB256(src, len, block);
	_BigPowMod(block, engine->RSA.pDecryptionKey, engine->RSA.pModulus, decrypted);
	_BigOutB256(decrypted, dst);

	_BigDestroy(block);
	_BigDestroy(decrypted);
}
