#include <iostream>
#include <memory>
#include <algorithm>

#include <cstdio>

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

template <typename BaseType, typename OffsetType, typename RetType>
inline RetType MAKE_PTR(BaseType Base, OffsetType Offset)
{
	return (RetType)(DWORD(Base) + DWORD(Offset));
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

PBYTE LoadFromDisk(char* filename)
{
	auto file = OpenFile(filename);
	auto size = GetFileSize(file, nullptr);
	auto buff = (PBYTE)VirtualAlloc(nullptr, size, MEM_COMMIT, PAGE_READWRITE);

	DWORD read;
	ReadFile(file, buff, size, &read, nullptr);
	CloseHandle(file);

	return buff;
}

typedef struct _LoadPE_CONTEXT
{
	/* Ptrs to headers */
    PIMAGE_DOS_HEADER      dos_hdr_ptr;
    PIMAGE_NT_HEADERS      pe_hdr_ptr;
	PIMAGE_SECTION_HEADER  sections;
    PIMAGE_BASE_RELOCATION reloc;

	/* Constants */
    DWORD sections_count;
	DWORD prefer_base_address;

	/*Real base address at which image is loaded */
    PBYTE real_base_addr;
} LoadPE_CONTEXT;

void* LoadPE_AllocateMemory(LoadPE_CONTEXT* ctx, PBYTE disk_image)
{
	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)(disk_image);
	PIMAGE_NT_HEADERS NtHeaders = (PIMAGE_NT_HEADERS)(disk_image + DosHeader->e_lfanew);

	void* mem = VirtualAlloc(nullptr /*(void*)NtHeaders->OptionalHeader.ImageBase*/,
                             NtHeaders->OptionalHeader.SizeOfImage,
                             MEM_COMMIT,
                             PAGE_READWRITE);
	ctx->real_base_addr = PBYTE(mem);
	return mem;
}

void LoadPE_LoadHeaders(LoadPE_CONTEXT* ctx, PBYTE disk_image)
{
	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)(disk_image);
	PIMAGE_NT_HEADERS NtHeaders = (PIMAGE_NT_HEADERS)(disk_image + DosHeader->e_lfanew);

	ctx->dos_hdr_ptr = DosHeader;
	ctx->pe_hdr_ptr  = NtHeaders;
	ctx->sections    = IMAGE_FIRST_SECTION(ctx->pe_hdr_ptr);
	ctx->sections_count = ctx->pe_hdr_ptr->FileHeader.NumberOfSections;
	ctx->prefer_base_address = ctx->pe_hdr_ptr->OptionalHeader.ImageBase;
	CopyMemory(ctx->real_base_addr, disk_image, NtHeaders->OptionalHeader.SizeOfHeaders);

	const DWORD relocs_rva = ctx->pe_hdr_ptr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
	if (relocs_rva)
	{
		ctx->reloc = PIMAGE_BASE_RELOCATION(ctx->real_base_addr + relocs_rva);
	}
	else
	{
		ctx->reloc = nullptr;
	}
}

void LoadPE_LoadSections(LoadPE_CONTEXT* ctx, PBYTE disk_image)
{
	for (int i = 0; i < ctx->sections_count; ++i)
	{
        std::size_t section_size = min(ctx->sections[i].Misc.VirtualSize, ctx->sections[i].SizeOfRawData);
        CopyMemory(ctx->real_base_addr + ctx->sections[i].VirtualAddress,
                   disk_image + ctx->sections[i].PointerToRawData,
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

void LoadPE_PerformRelocation(const LoadPE_CONTEXT* ctx)
{
	PBYTE pReloc = (PBYTE)ctx->reloc;
	std::cout << "delta: " << std::hex << ((DWORD)ctx->real_base_addr - ctx->prefer_base_address) << std::endl;

	while (true)
	{
		PBYTE pageRVA = (PBYTE)(*(DWORD *)pReloc);
		DWORD blockSize = *(DWORD *)(pReloc + 4);

		if (!blockSize) break;

		printf("FIXUP BLOCK - pageRVA: %06Xh, size %06d bytes\n"\
			"-------------------------------------------------\n",
			pageRVA, blockSize);

		for (int i = 8; i < blockSize; i += 2)
		{
			DWORD typeX = (*(WORD *)(pReloc + i)) >> 12;
			DWORD offsetX = (*(WORD *)(pReloc + i)) & ((1 << 12) - 1);

			switch (typeX)
			{
			case 0:
				std::cout << "\tIMAGE_REL_BASED_ABSOLUTE" << std::endl;
				break;
			case 3:
				std::cout << "\tIMAGE_REL_BASED_HIGHLOW "
					<< std::hex << (offsetX + ctx->prefer_base_address)
					<< "------->"
					<< std::hex << (offsetX + (DWORD)ctx->real_base_addr)
					<< std::endl;
				break;
			default:
				std::cout << "type " << std::hex << typeX << " not supported" << std::endl;
				break;
			}
		}
		std::cout << std::endl;
		pReloc += blockSize;
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
        (PIMAGE_IMPORT_DESCRIPTOR) (ctx->real_base_addr
                                    + ctx->pe_hdr_ptr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    for (; import_desc->Characteristics; ++import_desc)
    {
        const char* dll_name = (char*)(ctx->real_base_addr + import_desc->Name);
        std::cout << dll_name << std::endl;

		HMODULE hModule = LoadLibraryA(dll_name);

        PIMAGE_THUNK_DATA ThunkData = GetOriginalFirstThunk(ctx->real_base_addr, import_desc);
		for (PIMAGE_THUNK_DATA iat = GetFirstThunk(ctx->real_base_addr, import_desc); ThunkData->u1.Function; ++ThunkData, ++iat)
		{
			PIMAGE_IMPORT_BY_NAME symbol = PIMAGE_IMPORT_BY_NAME(ctx->real_base_addr + ThunkData->u1.Function);
			std::cout << (char*)symbol->Name << std::endl;

			FARPROC function = GetProcAddress(hModule, symbol->Name);
			std::cout << std::hex << DWORD(function) << std::endl;

			*(DWORD_PTR *)(ctx->real_base_addr + iat->u1.Function) = DWORD_PTR(function);
		}
    }
}

void LoadPE_CallEntryPoint(LoadPE_CONTEXT* ctx)
{
    DWORD EntryPoint = (DWORD)ctx->real_base_addr
                     + ctx->pe_hdr_ptr->OptionalHeader.AddressOfEntryPoint;
	__asm jmp[EntryPoint];
}

void LoadPE(PBYTE disk_image)
{
	LoadPE_CONTEXT ctx;

	LoadPE_AllocateMemory(&ctx, disk_image);
	LoadPE_LoadHeaders(&ctx, disk_image);
	LoadPE_LoadSections(&ctx, disk_image);
	LoadPE_PerformRelocation(&ctx);
	LoadPE_ResolveImport(&ctx);
	LoadPE_SetSectionMemoryProtection(&ctx);
	LoadPE_CallEntryPoint(&ctx);
}

int main()
{
	std::cout << ALIGN_UP(17, 16) << std::endl;
	std::cout << ALIGN_UP2(1, 16) << std::endl;

	std::size_t x = 17;
	std::cout << ALIGN_UP2(x, (std::size_t)16) << std::endl;

	PBYTE disk_image = LoadFromDisk("Thread.exe");
	LoadPE(disk_image);
}