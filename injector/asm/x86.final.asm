BITS 32

%define MAX_PATH		0x1
%define SZWCHAR			0x2 ; sizeof(wchar_t)
%define u(x)			__utf16__(x) 

entry:
	mov eax, [esp]
	ret
main:
	push dword 0xDEADBEEF 			; Store original context (Will be replaced in C)
	pushf
	pusha
	call entry
	add eax, wsDllPath - $			; EAX = L"procrewriter\0"
	push dword eax					; Stack: L"procrewriter\0", original context
	xor ecx, ecx					; ECX = 0
; Find base address of kernel32.dll. This code should work on Windows 5.0-7.0
	mov esi, [ fs:ecx + 0x30 ]		; ESI = &(PEB) ([FS:0x30])
	mov esi, [ esi + 0x0C ]			; ESI = PEB->Ldr
	mov esi, [ esi + 0x1C ]			; ESI = PEB->Ldr.InInitOrder (first module)
next_module:
	mov ebp, [ esi + 0x08 ]			; EBP = InInitOrder[X].base_address
	mov edi, [ esi + 0x20 ]			; EDI = InInitOrder[X].module_name (unicode string)
	mov esi, [ esi]					; ESI = InInitOrder[X].flink (next module)
	cmp [ edi + 12*SZWCHAR ], cl	; modulename[12] == 0 ? strlen("kernel32.dll") == 12
	jne next_module					; No: try next module.
begin_calls:
	mov edi, dword ebp				; EDI = kernel32.base_address
	add edi, dword 0xDEADBEEF		; EDI = kernel32.LoadLibraryW (LoadLibraryW offset found at runtime)
	call dword edi					; Stack: L"procrewriter\0", EBP, caller
	mov edi, eax					; EDI = procrewriter.base
	add edi, dword 0xDEADBEEF		; EDI = procrewriter.Initialize (Initialize offset found at runtime)
	call dword edi					; Call procrewriter.Initialize(ntdll)
	popa							; Restore original context.
	popf
	ret
wsDllPath:
	db u('injected_dll'), 0x0, 0x0
