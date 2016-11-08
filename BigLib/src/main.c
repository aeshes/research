#include <stdio.h>
#include "biglib.h"
#include "rsa.h"

int main(int argc, char *argv[])
{
	RSA_CONTEXT rsa;
	char data[] = "DEADFADE";
	char encrypted[256] = { 0 };
	char decrypted[256] = { 0 };

	RSASetup(&rsa);
	RSAEncrypt(&rsa, data, 8, encrypted);
	printf("encrypted = %s\n", encrypted);

	RSADecrypt(&rsa, encrypted, 584, decrypted);
	printf("decrypted = %s\n", decrypted);

	RSAClear(&rsa);
}
