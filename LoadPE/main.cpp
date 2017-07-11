#include <iostream>
#include <memory>
#include <algorithm>

#include <windows.h>

template <typename TInt>
TInt ALIGN_DOWN(TInt x, TInt align)
{
	return (x & ~(align - 1));
}

template <typename TInt>
TInt ALIGN_UP(TInt x, TInt align)
{
	return (x & (align - 1)) ? ALIGN_DOWN(x, align) + align : x;
}

template <typename TInt>
TInt ALIGN_UP2(TInt x, TInt align)
{
	return (x + (align - 1)) & -align;
}


HANDLE OpenFile(char* filename)
{
	HANDLE hFile = CreateFileA(filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);
	return hFile;
}

void* LoadFromDisk(char* filename)
{
	auto file = OpenFile(filename);
	auto size = GetFileSize(file, nullptr);
	auto buff = (char*)VirtualAlloc(nullptr, size, MEM_COMMIT, PAGE_READWRITE);

	DWORD read;
	ReadFile(file, buff, size, &read, nullptr);
	CloseHandle(file);

	return buff;
}

typedef struct _LoadPE_CONTEXT
{
	PIMAGE_DOS_HEADER dos_hdr_ptr;
	PIMAGE_NT_HEADERS pe_hdr_ptr;

	void* load_addr;
} LoadPE_CONTEXT;

void* LoadPE_AllocateMemory(LoadPE_CONTEXT* ctx, char* disk_image)
{
	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)(disk_image);
	PIMAGE_NT_HEADERS NtHeaders = (PIMAGE_NT_HEADERS)((char*)disk_image + DosHeader->e_lfanew);

	void* mem = VirtualAlloc(nullptr /*(void*)NtHeaders->OptionalHeader.ImageBase*/,
                             NtHeaders->OptionalHeader.SizeOfImage,
                             MEM_COMMIT,
                             PAGE_READWRITE);
	ctx->load_addr = mem;
	return mem;
}

void LoadPE_LoadHeaders(LoadPE_CONTEXT* ctx, void* disk_image)
{
	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)(disk_image);
	PIMAGE_NT_HEADERS NtHeaders = (PIMAGE_NT_HEADERS)((char*)disk_image + DosHeader->e_lfanew);

	ctx->dos_hdr_ptr = DosHeader;
	ctx->pe_hdr_ptr  = NtHeaders;
	CopyMemory(ctx->load_addr, disk_image, NtHeaders->OptionalHeader.SizeOfHeaders);
}

void LoadPE_LoadSections(LoadPE_CONTEXT* ctx, void* disk_image)
{
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ctx->pe_hdr_ptr);
	for (int i = 0; i < ctx->pe_hdr_ptr->FileHeader.NumberOfSections; ++i, ++section)
	{
		std::size_t section_size = min(section->Misc.VirtualSize, section->SizeOfRawData);
        CopyMemory((void*)((DWORD)ctx->load_addr + section->VirtualAddress),
                   (void*)((DWORD)disk_image + section->PointerToRawData),
                   section_size);
	}
}

void LoadPE(char* disk_image)
{
	LoadPE_CONTEXT ctx;

	LoadPE_AllocateMemory(&ctx, disk_image);
	LoadPE_LoadHeaders(&ctx, disk_image);
	LoadPE_LoadSections(&ctx, disk_image);
}

int main()
{
	std::cout << ALIGN_UP(17, 16) << std::endl;
	std::cout << ALIGN_UP2(1, 16) << std::endl;

	char* disk_image = (char*)LoadFromDisk("main.exe");
	LoadPE(disk_image);
}