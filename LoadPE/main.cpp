#include <iostream>
#include <memory>

#include <windows.h>


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

	CopyMemory(ctx->load_addr, disk_image, NtHeaders->OptionalHeader.SizeOfHeaders);
}

void LoadPE_LoadSections(LoadPE_CONTEXT* ctx, void* disk_image)
{

}

void LoadPE(char* disk_image)
{
	LoadPE_CONTEXT ctx;

	LoadPE_AllocateMemory(&ctx, disk_image);
	LoadPE_LoadHeaders(&ctx, disk_image);
}

int main()
{
	char* disk_image = (char*)LoadFromDisk("main.exe");
	LoadPE(disk_image);
}