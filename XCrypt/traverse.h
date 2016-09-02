#ifndef TRAVERSE_H
#define TRAVERSE_H

#include <string.h>
#include <windows.h>
#include <shlwapi.h>

int traverse_folders(char *path,
					 char *file_mask,
					 void(*proc)(char *file, void *xtea_data),
					 void *xtea_data);

#endif
