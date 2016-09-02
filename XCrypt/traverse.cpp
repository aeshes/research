#include "traverse.h"

int traverse_folders(char *path,
					 char *file_mask,
					 void (*proc)(char *file, void *xtea_data),
					 void *xtea_data)
{
	WIN32_FIND_DATA find_data;
	HANDLE file;
	char *last_char = path + lstrlen(path);

	lstrcat(path, file_mask);
	file = FindFirstFile(path, &find_data);
	*last_char = '\0';
	if (file == INVALID_HANDLE_VALUE)
		return FALSE;

	do
	{
		if (find_data.cFileName[0] == '.')
			continue;

		lstrcat(path, find_data.cFileName);
		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			lstrcat(path, "\\");
			traverse_folders(path, file_mask, proc, xtea_data);
		}
		else
			proc(path, xtea_data);
		*last_char = '\0';
	} while (FindNextFile(file, &find_data));

	FindClose(file);
	return 1;
}