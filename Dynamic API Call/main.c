#include <windows.h>
#include <stdio.h>

/*
	Calling API by hash of its name.
	Use cases: bypass detection by import table, slowdown reverse engineering.
	(c) aeshes. Copy me if you can.
*/

__declspec(naked) unsigned int GetKernel32(void)
{
	__asm
	{
		mov eax, fs:[0x30];		/* Get pointer to PEB */
		mov eax, [eax + 0xC];	/* Get pointer to PEB_LDR_DATA */
		mov eax, [eax + 0x14];	/* InMemoryOrderModuleList */

		mov eax, [eax];			/* Get the 2-nd entry */
		mov eax, [eax];			/* Get the 3-rd entry */
		mov eax, [eax + 0x10];	/* Get the 3-rd entry base address */

		ret;
	};
}

unsigned int Hash(char* str)
{
   unsigned int hash = 0x01b63a;

   for(; *str; str++)
   {
      hash = ((hash << 5) + hash) + (*str);
   }
   return hash;	
}

DWORD __stdcall GetAPI(const DWORD library, const DWORD APIHASH)
{
	if (library)
	{
		PIMAGE_DOS_HEADER dos_hdr = (PIMAGE_DOS_HEADER)library;
		PIMAGE_NT_HEADERS nt_hdr = (PIMAGE_NT_HEADERS)(library + dos_hdr->e_lfanew);
		PIMAGE_OPTIONAL_HEADER optional_hdr = &nt_hdr->OptionalHeader;
		PIMAGE_DATA_DIRECTORY data_directory = optional_hdr->DataDirectory;
		PIMAGE_EXPORT_DIRECTORY export = (PIMAGE_EXPORT_DIRECTORY)(library + data_directory[0].VirtualAddress);

		DWORD *names = (DWORD *)(library + export->AddressOfNames);
		WORD *ordinals = (WORD *)(library + export->AddressOfNameOrdinals);
		DWORD *functions = (DWORD *)(library + export->AddressOfFunctions);

		for (int i = 0; i < export->NumberOfNames; ++i)
		{
			char *name = (char *)(library + names[i]);
			if (Hash(name) == APIHASH)
				return functions[ordinals[i]] + (DWORD)library;
		}
	}

	return 0;
}

void * CallAPI(DWORD dwHash, DWORD nArgs, ...)
{
	va_list arg_list;
	char *shellcode = VirtualAlloc(NULL, 40, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	va_start(arg_list, nArgs);

	/* push arguments from right to left */
	for (int i = nArgs - 1; i >= 0; --i)
	{
		*(WORD *)	&shellcode[5 * i] = 0x68;	/* opcode of push*/
		*(DWORD *)	&shellcode[5 * i + 1] = va_arg(arg_list, DWORD);	/* what to push */
	}

	va_end(arg_list);

	*(WORD  *)	&shellcode[5 * nArgs] = 0xB8;	/* mov eax, value */
	*(DWORD *)	&shellcode[5 * nArgs + 1] = GetAPI(GetKernel32(), dwHash);
	*(WORD  *)	&shellcode[5 * nArgs + 5] = 0xD0FF;	/* call eax */
	*(WORD  *)	&shellcode[5 * nArgs + 7] = 0xC3;	/* ret */

	void * (*p)(void) = (void * (*)(void))shellcode;
	void * ret = p();
	VirtualFree(shellcode, 40, MEM_RELEASE);

	return ret;
}


typedef BOOL (WINAPI * noimport_beep) (DWORD, DWORD);

#define dwBeep 0xfa2c2f56
#define dwVirtualAlloc 0x7554284c

int main(int argc, char *argv[])
{
	CallAPI(dwBeep, 2, 700, 1400);
	CallAPI(dwVirtualAlloc, 4, 0, 40, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}
