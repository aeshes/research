#include <iostream>
#include <memory>
#include <algorithm>

#include <windows.h>

#include "def.h"

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


__declspec(naked) PPEB GetPEB()
{
	__asm
	{
		mov eax, fs:[0x30]
		ret
	}
}

void* LoadPE_AllocateMemory(LoadPE_CONTEXT* ctx, PBYTE disk_image)
{
	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)(disk_image);
	PIMAGE_NT_HEADERS NtHeaders = (PIMAGE_NT_HEADERS)(disk_image + DosHeader->e_lfanew);

	std::size_t ImageBase   = NtHeaders->OptionalHeader.ImageBase;
	std::size_t SizeOfImage = NtHeaders->OptionalHeader.SizeOfImage;
	UnmapViewOfFile((void *)ImageBase);
	VirtualFree((void *)ImageBase, SizeOfImage, MEM_DECOMMIT);
	VirtualFree((void *)ImageBase, SizeOfImage, MEM_RELEASE);
	void* mem = VirtualAlloc((void*)ImageBase,
                             SizeOfImage,
                             MEM_RESERVE | MEM_COMMIT,
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
	if (import_desk->OriginalFirstThunk == 0)
		return nullptr;
	else
		return PIMAGE_THUNK_DATA(image_base + import_desk->OriginalFirstThunk);
}

inline PIMAGE_THUNK_DATA GetFirstThunk(PBYTE image_base, PIMAGE_IMPORT_DESCRIPTOR import_desk)
{
	return PIMAGE_THUNK_DATA(image_base + import_desk->FirstThunk);
}

void LoadPE_WalkImportDescriptor(LoadPE_CONTEXT* ctx, PIMAGE_IMPORT_DESCRIPTOR pImportDesc)
{
	const char* pszDllName = (char *)(ctx->pbRealImageBase + pImportDesc->Name);

	HMODULE hModule = GetModuleHandleA(pszDllName);
	if (!hModule)
	{
		hModule = LoadLibraryA(pszDllName);
	}
	if (!hModule) return;

	PIMAGE_THUNK_DATA LookupFirstThunkEntry = GetOriginalFirstThunk(ctx->pbRealImageBase, pImportDesc);
	PIMAGE_THUNK_DATA IATFirstThunkEntry = GetFirstThunk(ctx->pbRealImageBase, pImportDesc);
	for (; LookupFirstThunkEntry->u1.AddressOfData; ++LookupFirstThunkEntry, ++IATFirstThunkEntry)
	{
		ULONG Function = 0;
		PIMAGE_IMPORT_BY_NAME symbol = PIMAGE_IMPORT_BY_NAME(ctx->pbRealImageBase + LookupFirstThunkEntry->u1.AddressOfData);
		if (IMAGE_SNAP_BY_ORDINAL32(LookupFirstThunkEntry->u1.Ordinal))
		{
			ULONGLONG Ordinal = IMAGE_ORDINAL32(LookupFirstThunkEntry->u1.Ordinal);
			Function = (ULONG)GetProcAddress(hModule, (char*)symbol->Hint);
		}
		else
		{
			Function = (ULONG)GetProcAddress(hModule, symbol->Name);
		}
		*(ULONG *)IATFirstThunkEntry = Function;
	}
}

void LoadPE_ResolveImport(LoadPE_CONTEXT* ctx)
{
    PIMAGE_IMPORT_DESCRIPTOR import_desc =
        (PIMAGE_IMPORT_DESCRIPTOR) (ctx->pbRealImageBase
                                    + ctx->pPeHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    for (; import_desc->Characteristics; ++import_desc)
    {
		LoadPE_WalkImportDescriptor(ctx, import_desc);
    }
}

void LoadPE_FixPEB(LoadPE_CONTEXT* ctx)
{
	PPEB Peb = GetPEB();
	Peb->ImageBaseAddress = ctx->pbRealImageBase;

	PLDR_DATA_TABLE_ENTRY pLdrEntry = PLDR_DATA_TABLE_ENTRY(Peb->Ldr->InLoadOrderModuleList.Flink);
	pLdrEntry->DllBase = ctx->pbRealImageBase;
	pLdrEntry = PLDR_DATA_TABLE_ENTRY(Peb->Ldr->InMemoryOrderModuleList.Flink);
	pLdrEntry->DllBase = ctx->pbRealImageBase;
	pLdrEntry = PLDR_DATA_TABLE_ENTRY(Peb->Ldr->InInitializationOrderModuleList.Flink);
	pLdrEntry->DllBase = ctx->pbRealImageBase;
}

void LoadPE_CallEntryPoint(LoadPE_CONTEXT* ctx)
{
    DWORD EntryPoint = (DWORD)ctx->pbRealImageBase
                     + ctx->pPeHdr->OptionalHeader.AddressOfEntryPoint;
	__asm call EntryPoint;
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
	LoadPE_FixPEB(&ctx);
	LoadPE_CallEntryPoint(&ctx);
}

int main()
{
	PBYTE disk_image = LoadFromDisk("calc.exe");
	LoadPE(disk_image);
}