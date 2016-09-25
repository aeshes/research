#include <stdio.h>
#include <windows.h>

__declspec(naked) DWORD GetKernel32(void)
{
	__asm
	{
		push esi
		mov eax, fs:[0x30]		; PEB base
		mov eax, [eax + 0x0C]		; goto PEB_LDR_DATA
		mov eax, [eax + 0x1C]		; first entry in the list of loaded modules	(struct LIST_ENTRY *Blink)
		mov eax, [eax]			; second entry
		mov eax, [eax]			; third entry (kernel32.dll)
		mov eax, [eax + 0x08]		; kernel32 base memory
		pop esi
		ret
	}
}

__declspec(naked) DWORD WINAPI FindGetProcAddr(DWORD kernel32)
{
	__asm
	{
		mov eax, [esp + 4]
		mov ebx, eax				; kernel32 base
		mov edi, dword ptr [ebx + 0x3C]		; PE header offset
		add edi, ebx				; add base address to offset
		mov edi, dword ptr [edi + 0x78]		; to export table
		add edi, ebx				; edi = export table
		mov esi, dword ptr [edi + 0x20]		; to export name table
		add esi, ebx
		;mov ecx, dword ptr [edi + 0x14]	; number of exported functions
		xor ecx, ecx
		get_function:
			inc ecx
			lodsd
			add eax, ebx
			cmp dword ptr[eax], 0x50746547		; compare with "GetP"
			jnz get_function
			cmp dword ptr[eax + 0x4], 0x41636f72	; compare with "rocA"
			jnz get_function
			cmp dword ptr[eax + 0x8], 0x65726464	; compare with "ddre"
			mov esi, [edi + 0x24]			; offset of ordinals
			add esi, ebx
			mov cx, [esi + ecx * 2]			; cx = number of function
			dec ecx
			mov esi, [edi + 0x1C]			; offset address table
			add esi, ebx				; esi = address table
			mov edi, [esi + ecx * 4]		; edi = pointer (offset)
			add edi, ebx				; edi = GetProcAddress
		mov eax, edi
		ret
	}
}

int main(int argc, char *argv[])
{
	DWORD kernel32 = GetKernel32();
	WINAPI (*f)(HMODULE, LPCSTR) = (void *)FindGetProcAddr(kernel32);
	FARPROC WINAPI win_exec = (FARPROC)f((void *)kernel32, "WinExec");

	win_exec("calc.exe", SW_SHOW);
	return 0;
}
