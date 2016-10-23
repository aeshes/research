#include "rsa.h"

char n[] = "2A8B1A50F22D22E11B809387052240060B35A16CD65570BED402A9BAD" \
		   "74D6370BC704F0B7F94E1BACF47FC10534740D399D334074672899647" \
		   "0BCC0BE0BDDB87500FD07CCA23BA48EDD";
char e[] = "10001";
char d[] = "A440297A1B86E802F28B9708D36948860D01221B71E6AD441485B3870" \
		   "9785F54023416475CFD091B87F59E170E4736E97E73B1BFA3AA3BE290" \
		   "2C8CF5E16B7B844AFBDFD261294BE241";

void RSASetup(RSA_CONTEXT *rsa)
{
	rsa->modulus = _BigCreate(0);
	rsa->encryption_key = _BigCreate(0);
	rsa->decryption_key = _BigCreate(0);

	_BigIn(n, 16, rsa->modulus);
	_BigIn(e, 16, rsa->encryption_key);
	_BigIn(d, 16, rsa->decryption_key);
}

void RSAClear(RSA_CONTEXT *rsa)
{
	_BigDestroy(rsa->modulus);
	_BigDestroy(rsa->encryption_key);
	_BigDestroy(rsa->decryption_key);
}

void RSAEncrypt(RSA_CONTEXT *rsa, char *src, DWORD len, char *dst)
{
	DWORD block = _BigCreate(0);
	DWORD encrypted = _BigCreate(0);

	_BigInB256(src, len, block);
	_BigPowMod(block, rsa->encryption_key, rsa->modulus, encrypted);
	_BigOutB16(encrypted, dst);

	_BigDestroy(block);
	_BigDestroy(encrypted);
}

void RSADecrypt(RSA_CONTEXT *rsa, char *src, DWORD len, char *dst)
{
	DWORD block = _BigCreate(0);
	DWORD decrypted = _BigCreate(0);

	_BigIn(src, 16, block);
	_BigPowMod(block, rsa->decryption_key, rsa->modulus, decrypted);
	_BigOutB256(decrypted, dst);

	_BigDestroy(block);
	_BigDestroy(decrypted);
}
