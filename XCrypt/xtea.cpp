#include "xtea.h"
#include "crc32.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

void xtea_init(XTEA_DATA *xtea_data, int key_size, int key_range)
{
	xtea_data->KEY_SIZE = key_size;
	xtea_data->XTEA_KEY_RANGE = key_range;

	//gen_xtea_key(xtea_data);
	CopyMemory(xtea_data->xtea_key, "abcdabcdabcdabcd", 16);

	xtea_data->XTEA_KEY_HASH = crc32(0xFFFFFFFF, xtea_data->xtea_key, xtea_data->KEY_SIZE);
}

void gen_xtea_key(XTEA_DATA *xtea)
{
	int i;

	srand(time(NULL));
	for (i = 0; i < 16; i++)
	{
		if (i < xtea->KEY_SIZE) xtea->xtea_key[i] = rand() % xtea->XTEA_KEY_RANGE + 'A';
		else xtea->xtea_key[i] = 0;
		printf("c = %c, d = %u\n", xtea->xtea_key[i], xtea->xtea_key[i]);
	}
}

void xtea_encrypt(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4])
{
	unsigned int i;
	uint32_t v0 = v[0], v1 = v[1], sum = 0, delta = 0x9E3779B9;
	for (i = 0; i < num_rounds; i++)
	{
		v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
		sum += delta;
		v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
	}
	v[0] = v0; v[1] = v1;
}

void xtea_decrypt(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4])
{
	unsigned int i;
	uint32_t v0 = v[0], v1 = v[1], delta = 0x9E3779B9, sum = delta*num_rounds;
	for (i = 0; i < num_rounds; i++)
	{
		v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
		sum -= delta;
		v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
	}
	v[0] = v0; v[1] = v1;
}