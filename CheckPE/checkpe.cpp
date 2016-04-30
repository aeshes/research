// Program checks is given file a PE-file
// Usage: checkpe.exe filename

#include <iostream>
#include <fstream>
#include <Windows.h>

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cout << "Usage: checkpe.exe <filename>" << std::endl;
		return 0;
	}

	// Open PE file as binary
	std::ifstream pefile;
	pefile.open(argv[1], std::ios::in | std::ios::binary);
	if (!pefile.is_open())
	{
		std::cout << "Unable to open file" << std::endl;
		return 0;
	}

	// Determine size of the PE file
	pefile.seekg(0, std::ios::end);
	std::streamoff nFileLength = pefile.tellg();
	pefile.seekg(0);

	// If length of file is less than summary size of main structures
	// then this is not PE file
	if (nFileLength <= sizeof(IMAGE_DOS_HEADER) +
					   sizeof(IMAGE_NT_HEADERS))
	{
		std::cout << "File length " << nFileLength 
				  << " is less than sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS)..." 
				  << std::endl;
		return 0;
	}

	IMAGE_DOS_HEADER dos_header;
	pefile.read(reinterpret_cast<char*> (&dos_header), sizeof(IMAGE_DOS_HEADER));
	if (pefile.bad() | pefile.eof())
	{
		std::cout << "Unable to read IMAGE_DOS_HEADER..." << std::endl;
		return 0;
	}

	// Try to test is this file a real PE file
	std::cout << "Try to test if signature is a DOS signature..." << std::endl;
	if (dos_header.e_magic != IMAGE_DOS_SIGNATURE)
	{
		std::cout << "File is not a DOS executable file..." << std::endl;
		return 0;
	}

	// e_lfarlc must be greater or equal to 0x40
	if (dos_header.e_lfarlc < 0x40)
	{
		std::cout << "e_lfarlc field is less than 0x40..." << std::endl;
		return 0;
	}

	// Field e_lfanew contains offset of PE-header
	// Jump to IMAGE_NT_HEADERS struct
	pefile.seekg(dos_header.e_lfanew);

	IMAGE_NT_HEADERS nt_headers;
	pefile.read(reinterpret_cast<char*>(&nt_headers), sizeof(IMAGE_NT_HEADERS));
	if (pefile.bad() | pefile.fail())
	{
		std::cout << "Unable to reach IMAGE_NT_HEADERS..." << std::endl;
		return 0;
	}

	std::cout << "Try to test if signature is a PE  signature..." << std::endl;
	if (nt_headers.Signature != IMAGE_NT_SIGNATURE)
	{
		std::cout << "File is not a PE file..." << std::endl;
	}

	// All the conditions are satisfied
	std::cout << "File is a real PE file." << std::endl;
	return 0;
} 