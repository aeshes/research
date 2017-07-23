#include <iostream>
#include <memory>
#include <algorithm>

#include <cstdio>

#include <windows.h>

#pragma warning ( disable : 4146 )


template <typename TInt, typename TAlign = std::size_t>
TInt ALIGN_UP(TInt x, TAlign align)
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
    PIMAGE_DOS_HEADER      pDosHdr;
    PIMAGE_NT_HEADERS      pPeHdr;
    PIMAGE_SECTION_HEADER  pSections;
    PIMAGE_BASE_RELOCATION pRelocs;

    /* Constants */
    DWORD dwSectionCount;
    DWORD dwPreferImageBase;
    DWORD dwRelocDirSize;

    /*Real base address at which image is loaded */
    PBYTE pbRealImageBase;
} LoadPE_CONTEXT;

typedef struct _IMAGE_FIXUP_ENTRY
{
    WORD Offset : 12;
    WORD Type   : 4;
} IMAGE_FIXUP_ENTRY, *PIMAGE_FIXUP_ENTRY;

void* LoadPE_AllocateMemory(LoadPE_CONTEXT* ctx, PBYTE disk_image)
{
	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)(disk_image);
	PIMAGE_NT_HEADERS NtHeaders = (PIMAGE_NT_HEADERS)(disk_image + DosHeader->e_lfanew);

	void* mem = VirtualAlloc(nullptr /*(void*)NtHeaders->OptionalHeader.ImageBase*/,
                             NtHeaders->OptionalHeader.SizeOfImage,
                             MEM_COMMIT,
                             PAGE_READWRITE);
	ctx->pbRealImageBase = PBYTE(mem);
	return mem;
}

WORD LoadPE_GetRealNumberOfSections(PIMAGE_NT_HEADERS pNtHdr)
{
	WORD iRealNumOfSect = 0;
	PIMAGE_SECTION_HEADER pSectionHdr = nullptr;

	__try
	{
		pSectionHdr = IMAGE_FIRST_SECTION(pNtHdr);
		for (int i = 0; i < pNtHdr->FileHeader.NumberOfSections; ++i)
		{
			if (!pSectionHdr->PointerToRelocations
				&& !pSectionHdr->PointerToLinenumbers
				&& !pSectionHdr->NumberOfRelocations
				&& !pSectionHdr->NumberOfLinenumbers
				&& pSectionHdr->Characteristics)
			{
				++iRealNumOfSect;
			}
			else
				return iRealNumOfSect;

			++pSectionHdr;
		}
		return iRealNumOfSect;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return 0;
	}
}

void LoadPE_LoadHeaders(LoadPE_CONTEXT* ctx, PBYTE disk_image)
{
	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)(disk_image);
	PIMAGE_NT_HEADERS NtHeaders = (PIMAGE_NT_HEADERS)(disk_image + DosHeader->e_lfanew);

	ctx->pDosHdr = DosHeader;
	ctx->pPeHdr  = NtHeaders;
	ctx->pSections    = IMAGE_FIRST_SECTION(ctx->pPeHdr);
	ctx->dwSectionCount = LoadPE_GetRealNumberOfSections(NtHeaders);
	ctx->dwPreferImageBase = ctx->pPeHdr->OptionalHeader.ImageBase;
	CopyMemory(ctx->pbRealImageBase, disk_image, NtHeaders->OptionalHeader.SizeOfHeaders);

	const DWORD relocs_rva = ctx->pPeHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
	if (relocs_rva)
	{
		ctx->pRelocs = PIMAGE_BASE_RELOCATION(ctx->pbRealImageBase + relocs_rva);
		ctx->dwRelocDirSize = ctx->pPeHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
	}
	else
	{
		ctx->pRelocs = nullptr;
		ctx->dwRelocDirSize = 0;
	}
}

void LoadPE_LoadSections(LoadPE_CONTEXT* ctx, PBYTE disk_image)
{
	for (int i = 0; i < ctx->dwSectionCount; ++i)
	{
        //DONT FORGET: Review virtual size calculation
        std::size_t aligned_size = ALIGN_UP(ctx->pSections[i].Misc.VirtualSize, ctx->pPeHdr->OptionalHeader.SectionAlignment); 
        std::size_t section_size = min(aligned_size, ctx->pSections[i].SizeOfRawData);
        CopyMemory(ctx->pbRealImageBase + ctx->pSections[i].VirtualAddress,
                   disk_image + ctx->pSections[i].PointerToRawData,
                   section_size);
	}
}

void LoadPE_SetSectionMemoryProtection(LoadPE_CONTEXT* ctx)
{
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ctx->pPeHdr);
	DWORD dwProtection = 0;
	for (int i = 0; i < ctx->pPeHdr->FileHeader.NumberOfSections; ++i, ++section)
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

		void* lpSectionAddress = (void*)((DWORD)ctx->pbRealImageBase + section->VirtualAddress);
		VirtualProtect(lpSectionAddress,
			section->Misc.VirtualSize,
			dwProtection,
			&dwProtection);
	}
}

inline void LoadPE_PatchAddress(const LoadPE_CONTEXT* ctx, PIMAGE_BASE_RELOCATION Reloc, PIMAGE_FIXUP_ENTRY Fixup, DWORD Delta)
{
	DWORD_PTR* Address = (DWORD_PTR *)(ctx->pbRealImageBase + Reloc->VirtualAddress + Fixup->Offset);
	*Address += Delta;
}

void LoadPE_PerformRelocation(const LoadPE_CONTEXT* ctx)
{
	DWORD Delta = (DWORD)ctx->pbRealImageBase - ctx->dwPreferImageBase;

	PIMAGE_BASE_RELOCATION Reloc = ctx->pRelocs;
	const DWORD RelocDirSize = ctx->dwRelocDirSize;
	DWORD Offset = 0;

	while (Offset != RelocDirSize)
	{
		const DWORD FixupCount = (Reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
		const PIMAGE_FIXUP_ENTRY Fixup = PIMAGE_FIXUP_ENTRY((DWORD(Reloc) + sizeof(IMAGE_BASE_RELOCATION)));
		for (int i = 0; i < FixupCount; ++i)
		{
			if (Fixup[i].Type == IMAGE_REL_BASED_HIGHLOW)
			{
				LoadPE_PatchAddress(ctx, Reloc, &Fixup[i], Delta);
			}
		}
		Offset += Reloc->SizeOfBlock;
		Reloc = PIMAGE_BASE_RELOCATION(DWORD(Reloc) + Reloc->SizeOfBlock);
	}
}

inline PIMAGE_THUNK_DATA GetOriginalFirstThunk(PBYTE image_base, PIMAGE_IMPORT_DESCRIPTOR import_desk)
{
	return PIMAGE_THUNK_DATA(image_base + import_desk->OriginalFirstThunk);
}

inline PIMAGE_THUNK_DATA GetFirstThunk(PBYTE image_base, PIMAGE_IMPORT_DESCRIPTOR import_desk)
{
	return PIMAGE_THUNK_DATA(image_base + import_desk->FirstThunk);
}

void LoadPE_ResolveImport(LoadPE_CONTEXT* ctx)
{
    PIMAGE_IMPORT_DESCRIPTOR import_desc =
        (PIMAGE_IMPORT_DESCRIPTOR) (ctx->pbRealImageBase
                                    + ctx->pPeHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    for (; import_desc->Characteristics; ++import_desc)
    {
        const char* dll_name = (char*)(ctx->pbRealImageBase + import_desc->Name);

		HMODULE hModule = GetModuleHandleA(dll_name);
		if (!hModule)
		{
			hModule = LoadLibraryA(dll_name);
		}
		if (!hModule) return;

        PIMAGE_THUNK_DATA LookupTable = GetOriginalFirstThunk(ctx->pbRealImageBase, import_desc);
		for (PIMAGE_THUNK_DATA iat = GetFirstThunk(ctx->pbRealImageBase, import_desc); LookupTable->u1.Function; ++LookupTable, ++iat)
		{
			ULONG function = 0;
			PIMAGE_IMPORT_BY_NAME symbol = PIMAGE_IMPORT_BY_NAME(ctx->pbRealImageBase + LookupTable->u1.AddressOfData);
			if (symbol->Name[0])
			{
				function = (ULONG)GetProcAddress(hModule, symbol->Name);
			}
			else
			{
				function = (ULONG)GetProcAddress(hModule, (char*)symbol->Hint);
			}

			*(ULONG *)iat = function;
		}
    }
}

void LoadPE_CallEntryPoint(LoadPE_CONTEXT* ctx)
{
    DWORD EntryPoint = (DWORD)ctx->pbRealImageBase
                     + ctx->pPeHdr->OptionalHeader.AddressOfEntryPoint;
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


	PBYTE disk_image = LoadFromDisk("main.exe");
	LoadPE(disk_image);
}