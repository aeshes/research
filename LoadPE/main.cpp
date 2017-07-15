#include <iostream>
#include <memory>
#include <algorithm>

#include <windows.h>

#pragma warning ( disable : 4146 )


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

template <typename TInt, typename TAlign = std::size_t>
TInt ALIGN_UP2(TInt x, TAlign align)
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

	void* real_base_addr;
} LoadPE_CONTEXT;

void* LoadPE_AllocateMemory(LoadPE_CONTEXT* ctx, char* disk_image)
{
	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)(disk_image);
	PIMAGE_NT_HEADERS NtHeaders = (PIMAGE_NT_HEADERS)((char*)disk_image + DosHeader->e_lfanew);

	void* mem = VirtualAlloc(nullptr /*(void*)NtHeaders->OptionalHeader.ImageBase*/,
                             NtHeaders->OptionalHeader.SizeOfImage,
                             MEM_COMMIT,
                             PAGE_READWRITE);
	ctx->real_base_addr = mem;
	return mem;
}

void LoadPE_LoadHeaders(LoadPE_CONTEXT* ctx, void* disk_image)
{
	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)(disk_image);
	PIMAGE_NT_HEADERS NtHeaders = (PIMAGE_NT_HEADERS)((char*)disk_image + DosHeader->e_lfanew);

	ctx->dos_hdr_ptr = DosHeader;
	ctx->pe_hdr_ptr  = NtHeaders;
	CopyMemory(ctx->real_base_addr, disk_image, NtHeaders->OptionalHeader.SizeOfHeaders);
}

void LoadPE_LoadSections(LoadPE_CONTEXT* ctx, void* disk_image)
{
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ctx->pe_hdr_ptr);
	for (int i = 0; i < ctx->pe_hdr_ptr->FileHeader.NumberOfSections; ++i, ++section)
	{
        std::size_t section_size = min(section->Misc.VirtualSize, section->SizeOfRawData);
        CopyMemory((void*)((DWORD)ctx->real_base_addr + section->VirtualAddress),
                   (void*)((DWORD)disk_image + section->PointerToRawData),
                   section_size);
	}
}

void LoadPE_SetSectionMemoryProtection(LoadPE_CONTEXT* ctx)
{
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ctx->pe_hdr_ptr);
	DWORD dwProtection = 0;
	for (int i = 0; i < ctx->pe_hdr_ptr->FileHeader.NumberOfSections; ++i, ++section)
	{
		switch (section->Characteristics)
		{
		case IMAGE_SCN_MEM_EXECUTE:
			dwProtection = PAGE_EXECUTE;
			break;
		case IMAGE_SCN_MEM_READ:
			dwProtection = PAGE_READONLY;
			break;
		case IMAGE_SCN_MEM_WRITE:
			dwProtection = PAGE_READWRITE;
			break;
		default:
			dwProtection = PAGE_EXECUTE_READWRITE;
			break;
		}

		void* lpSectionAddress = (void*)((DWORD)ctx->real_base_addr + section->VirtualAddress);
		VirtualProtect(lpSectionAddress,
			section->Misc.VirtualSize,
			dwProtection,
			&dwProtection);
	}
}

PIMAGE_THUNK_DATA GetOriginalFirstThunk(void* image_base, PIMAGE_IMPORT_DESCRIPTOR import_desk)
{
	return PIMAGE_THUNK_DATA((DWORD)image_base + import_desk->OriginalFirstThunk);
}

PIMAGE_THUNK_DATA GetFirstThunk(void* image_base, PIMAGE_IMPORT_DESCRIPTOR import_desk)
{
	return PIMAGE_THUNK_DATA((DWORD)image_base + import_desk->FirstThunk);
}

void LoadPE_ResolveImport(LoadPE_CONTEXT* ctx)
{
    PIMAGE_IMPORT_DESCRIPTOR import_desc =
        (PIMAGE_IMPORT_DESCRIPTOR) ((DWORD)ctx->real_base_addr
                                    + ctx->pe_hdr_ptr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    for (; import_desc->Characteristics; ++import_desc)
    {
        char* dll_name = (char*)((DWORD)ctx->real_base_addr + import_desc->Name);
        std::cout << dll_name << std::endl;

        // Some khuita (какая-то хуита, переделать)
        PIMAGE_THUNK_DATA ThunkData = GetOriginalFirstThunk(ctx->real_base_addr, import_desc);
        PIMAGE_IMPORT_BY_NAME name = PIMAGE_IMPORT_BY_NAME(ThunkData->u1.AddressOfData);
        std::cout << (char*)((DWORD)ctx->real_base_addr + name->Name) << std::endl;
    }
}

void LoadPE_CallEntryPoint(LoadPE_CONTEXT* ctx)
{
    DWORD EntryPoint = (DWORD)ctx->real_base_addr
                     + ctx->pe_hdr_ptr->OptionalHeader.AddressOfEntryPoint;
	__asm jmp[EntryPoint];
}

void LoadPE(char* disk_image)
{
	LoadPE_CONTEXT ctx;

	LoadPE_AllocateMemory(&ctx, disk_image);
	LoadPE_LoadHeaders(&ctx, disk_image);
	LoadPE_LoadSections(&ctx, disk_image);
	LoadPE_SetSectionMemoryProtection(&ctx);
	LoadPE_ResolveImport(&ctx);
	//LoadPE_CallEntryPoint(&ctx);
}

int main()
{
	std::cout << ALIGN_UP(17, 16) << std::endl;
	std::cout << ALIGN_UP2(1, 16) << std::endl;

	std::size_t x = 17;
	std::cout << ALIGN_UP2(x, (std::size_t)16) << std::endl;

	char* disk_image = (char*)LoadFromDisk("main.exe");
	LoadPE(disk_image);
}