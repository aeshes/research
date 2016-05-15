// Displays information about the PE file given

#include <stdio.h>
#include <string.h>
#include <stddef.h>	// for offsetof
#include <windows.h>

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf ("Usage: filename\n");
		return 0;
	}

	TCHAR *filename = argv[1];
	HANDLE hFile = CreateFile (filename, FILE_ALL_ACCESS, 0, 0,
				   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		perror ("Cant open file given\n");
		return 0;
	}

	DWORD dwFileSize = GetFileSize(hFile, NULL);
	if (dwFileSize == INVALID_FILE_SIZE)
	{
  		perror ("GetFileSize failed\n");
  		CloseHandle(hFile);
  		return 0;
	}

	HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hMapping == NULL)
	{
		perror ("CreateFileMapping failed\n");
		CloseHandle(hFile);
		return 0;
	}

	unsigned char *data_ptr = (unsigned char*) MapViewOfFile(hMapping,
	                                                       FILE_MAP_READ,
	                                                       0,
	                                                       0,
	                                                       dwFileSize);
	if (data_ptr == NULL)
	{
	 	perror ("MapViewOfFile failed\n");
	 	CloseHandle(hMapping);
		CloseHandle(hFile);
		return 0;
	}

	PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER) data_ptr;

	if (dos_header->e_magic != 'ZM')
	{

		perror ("Incorrect DOS signature\n");
	 	CloseHandle(hMapping);
		CloseHandle(hFile);
		return 0;
	}

	PIMAGE_NT_HEADERS nt_headers = (PIMAGE_NT_HEADERS) (data_ptr + dos_header->e_lfanew);

	if (nt_headers->Signature != 'EP')
	{
		perror ("Incorrect PE signature\n");
	 	CloseHandle(hMapping);
		CloseHandle(hFile);
		return 0;
	}

	PIMAGE_SECTION_HEADER section_header = (PIMAGE_SECTION_HEADER) (data_ptr +
									dos_header->e_lfanew +
									offsetof(IMAGE_NT_HEADERS, OptionalHeader) +
									nt_headers->FileHeader.SizeOfOptionalHeader);
	int i;
	for (i = 0; i < nt_headers->FileHeader.NumberOfSections; i++, section_header++)
	{
		char name[9] = {0};
		memcpy (name, section_header->Name, 8);
		printf ("Section name: %s\n=======================\n", name);
		printf ("Virtual size : 0x%x\n", section_header->Misc.VirtualSize);
		printf ("Raw size: 0x%x\n", section_header->SizeOfRawData);
		printf ("Virtual address: 0x%x\n", section_header->VirtualAddress);
		printf ("Raw address: 0x%x\n", section_header->PointerToRawData);
		
		printf ("Characteristics: ");
		if (section_header->Characteristics & IMAGE_SCN_MEM_READ)
			printf ("R ");
		if (section_header->Characteristics & IMAGE_SCN_MEM_WRITE)
			printf ("W ");
		if (section_header->Characteristics & IMAGE_SCN_MEM_EXECUTE)
			printf ("X ");
		if (section_header->Characteristics & IMAGE_SCN_MEM_DISCARDABLE)
			printf ("discardable ");
		if (section_header->Characteristics & IMAGE_SCN_MEM_SHARED)
			printf ("shared");
		printf ("\n\n");
	}

	UnmapViewOfFile (data_ptr);
	CloseHandle (hMapping);
	CloseHandle (hFile);
	return 0;
}
