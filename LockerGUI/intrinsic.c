#include "intrinsic.h"

/*#pragma function(atoi, memset)

int __cdecl atoi(char *p)
{
	int result = 0;
	while (*p)
	{
		result = result * 10 + (*p) - '0';
		p++;
	}
	return result;
}

void * __cdecl memset(void *pTarget, int value, unsigned int cbTarget) {
	unsigned char *p = (unsigned char *)pTarget;
	while (cbTarget-- > 0)
	{
		*p++ = (unsigned char)value;
	}
	return pTarget;
}*/