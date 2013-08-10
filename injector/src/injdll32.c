/**
 * @file injdll32_64.c
 * @brief thegod
 */
#include "precompiled.h"
#include "util.h"
#include "injector.h"

#ifdef BUILD_ARCH_X64
#	ifndef WOW64_CONTEXT_ALL
#		include "wow64.h"
		TWow64GetThreadContext Wow64GetThreadContext;
		TWow64SetThreadContext Wow64SetThreadContext;
#		define IMPORT_WOW64
#	endif
#	define CONTEXT WOW64_CONTEXT
#	undef CONTEXT_CONTROL
#	define CONTEXT_CONTROL  WOW64_CONTEXT_CONTROL
#	define GetThreadContext Wow64GetThreadContext
#	define SetThreadContext Wow64SetThreadContext
#endif

#ifdef IMPORT_WOW64
#	define GETPROC(proc) proc = (T##proc)GetProcAddress(hKernel, #proc)
static inline void import_wow64_context_procs(void)
{
	if (Wow64GetThreadContext == 0) {
		HMODULE hKernel = GetModuleHandleW(L"kernel32.dll");
		GETPROC(Wow64GetThreadContext);
		GETPROC(Wow64SetThreadContext);
		// Assume if one is defined, so is the other.
		if(Wow64GetThreadContext == 0) {
			fatal(1, L"Failed to get pointer to Wow64GetThreadContext.");
			return;
		}
	}
}
#	undef GETPROC
#else
#	define import_wow64_context_procs() 
#endif

#define CODESIZE 68
void inject_x86(LPPROCESS_INFORMATION ppi, wchar_t* sDllPath, size_t szDllPath)
{
	LPVOID mem;
	CONTEXT context;
	DWORD dwCodeSize, offLoadLibraryW, offInitFunc, mem32;
	BYTE code[CODESIZE + WSIZE(MAX_PATH+1)] = {0};
	union {
		PBYTE  pB;
		PWORD  pW;
		PDWORD pL;
	} ip;

	import_wow64_context_procs();
	dwCodeSize = CODESIZE + WSIZE(szDllPath + 1);
	offLoadLibraryW = find_export_x86(L"kernel32.dll", "LoadLibraryW");
	offInitFunc = find_export_x86(sDllPath, "Init");
	
	context.ContextFlags = CONTEXT;
	GetThreadContext(ppi->hThread, &context);
	
	mem = VirtualAllocEx( ppi->hProcess, NULL, dwCodeSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	mem32 = ptr_to_dw(mem);

	ip.pB = code;

	/** <label>entry</label> */
	*ip.pB++ = 0x8b;			// mov eax, [esp]
	*ip.pW++ = 0x2404;
	*ip.pB++ = 0xc3;			// ret

	/** <label>main</label> */
	*ip.pB++ = 0x68;			// push eip
	*ip.pL++ = context.Eip;		//   store the original context
	*ip.pB++ = 0x9c;			// pushf
	*ip.pB++ = 0x60;			// pusha
	*ip.pB++ = 0xe8;			// call entry
	*ip.pL++ = 0xfffffff0L;
	*ip.pB++ = 0x83;			// add eax, wsDllPath - $
	*ip.pW++ = 0x34c0;
	*ip.pB++ = 0x50;			// push eax
	//   stack = [ dllpath, original context ]
	*ip.pB++ = 0x31;			// xor ecx,ecx (ECX = 0)
	*ip.pB++ = 0xc9;			//   ecx = 0
	*ip.pB++ = 0x64;			// mov esi, [ fs:ecx + 0x30 ]
	*ip.pB++ = 0x8b;			//   ecx = &PEB ([fs:0x30])
	*ip.pB++ = 0x71;
	*ip.pB++ = 0x30;
	*ip.pB++ = 0x8b;			// mov esi, [ esi + 0x0C ]
	*ip.pW++ = 0xc76;			//   esi = PEB->Ldr
	*ip.pB++ = 0x8b;			// mov esi, [ esi + 0x1C ]
	*ip.pW++ = 0x1c76;			//   esi = PEB->Ldr.InInitOrder (first module)

	/** <label>nextmod</label> */
	*ip.pB++ = 0x8b;			// mov ebp, [ esi + 0x08 ]
	*ip.pW++ = 0x86e;			//   ebp = InInitOrder[X].base_address
	*ip.pB++ = 0x8b;			// mov edi, [ esi + 0x20 ]
	*ip.pW++ = 0x207e;			//   edi = InInitOrder[X].module_name (unicode string)
	*ip.pB++ = 0x8b;			// mov esi, [ esi]
	*ip.pB++ = 0x36;			//   esi = InInitOrder[X].flink (next module)
	*ip.pB++ = 0x38;			// cmp [ edi + 12*SZWCHAR ], cl
	*ip.pW++ = 0x184f;			//   modulename[12] == 0 ? strlen("kernel32.dll") == 12
	*ip.pB++ = 0x75;			// jne nextmod
	*ip.pB++ = 0xf3;			//   continue until we find kernel32

	*ip.pB++ = 0x89;			// mov edi, ebp
	*ip.pB++ = 0xef;			//   edi = kernel32.base_address
	*ip.pB++ = 0x81;			// add edi, LoadLibraryW offset
	*ip.pB++ = 0xc7;			//   edi = kernel32.LoadLibraryW
	*ip.pL++ = offLoadLibraryW;
	*ip.pB++ = 0xff;			// call edi
	*ip.pB++ = 0xd7;			//   stack = [ dllpath, ebp, caller ]
	*ip.pB++ = 0x89;			// mov edi, eax
	*ip.pB++ = 0xc7;			//   edi = dll.base
	*ip.pB++ = 0x81;			// add edi, dll.init_func offset
	*ip.pB++ = 0xc7;			//   edi = dll.init_func
	*ip.pL++ = offInitFunc;
	*ip.pB++ = 0xff;			// call DWORD edi
	*ip.pB++ = 0xd7;			//   call dll.init_func()
	*ip.pB++ = 0x61;			// popa
	*ip.pB++ = 0x9d;			// popf
	*ip.pB++ = 0xc3;			// ret
	
	CopyMemory((void*)ip.pB, sDllPath, WSIZE(szDllPath+1));
	WriteProcessMemory(ppi->hProcess, mem, code, dwCodeSize, NULL);
	FlushInstructionCache(ppi->hProcess, mem, dwCodeSize);
	context.Eip = mem32 + 4;
	SetThreadContext( ppi->hThread, &context );
}
