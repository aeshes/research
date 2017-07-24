.386
.model flat, stdcall
option casemap:none

include windows.inc
include kernel32.inc
include user32.inc
includelib kernel32.lib
includelib user32.lib


.data
	caption		db "ImageBase", 0
	format		db "0x%08x", 0
	buffer		db 256 dup(?)
.code

Main PROC
	invoke GetModuleHandleA, NULL
	invoke wsprintf, offset buffer, offset format, eax
	invoke MessageBoxA, NULL, offset buffer, offset caption, MB_OK
	invoke ExitProcess, 0
Main ENDP

end Main