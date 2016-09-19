#ifndef XTEA_H
#define XTEA_H

#include <stdint.h>

#define XTEA_GENERATE_LETTERS	58
#define XTEA_STD_KEYSIZE	16
#define XTEA_BLOCK_SIZE_BYTES	8
#define BLOCKS_TO_ENCRYPT	4

typedef struct XTEA_DATA
{
	int KEY_SIZE;
	int XTEA_KEY_RANGE;
	uint32_t XTEA_KEY_HASH;
	char xtea_key[16];
} XTEA_DATA, *PXTEA_DATA;


void xtea_init(XTEA_DATA *xtea_data, int key_size, int key_range);

void gen_xtea_key(XTEA_DATA *xtea_data);

/* take 64 bits of data in v[0] and v[1] and 128 bits of key[0] - key[3] */

void xtea_encrypt(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]);

void xtea_decrypt(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]);

#endif
