#include "precompiled.h"

static HMODULE hDll = NULL;
#if 0
void JmpPatch(void *pDest, void *pSrc, int nNops) {
	DWORD OldProt;
	int i = 0;
	VirtualProtect(pSrc, 5 + nNops, PAGE_EXECUTE_READWRITE, &OldProt);
	*(char*)pSrc = (char)0xE9;
	*(DWORD*)((DWORD)pSrc + 1) = (DWORD)pDest - (DWORD)pSrc - 5;
	for (i = 0; i < nNops; ++i) { *(BYTE*)((DWORD)pSrc + 5 + i) = 0x90; }
	VirtualProtect(pSrc, 5 + nNops, OldProt, &OldProt);
}
#endif

#ifdef BUILD_ARCH_X64
//	Do format strings use hex or decimal to describe the leading zeroes?
//	I should probably check..
#	define ADDR_FMT "0x%10X"
#else
#	define ADDR_FMT "0x%08X"
#endif

void WINAPI Init(void)
{
	printf("%s:\n", __FUNCTION__);
	printf("   hDllStr -> " ADDR_FMT "\n", hDll);
}

// How bored am I? So bored.
#define PRINT_REASON(DW,CODE) \
	case DW: \
		CODE; \
		printf(#DW "\n"); \
		break;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason) {
		PRINT_REASON(DLL_PROCESS_ATTACH, hDll = hModule)
		PRINT_REASON(DLL_THREAD_ATTACH, )
		PRINT_REASON(DLL_THREAD_DETACH, )
		PRINT_REASON(DLL_PROCESS_DETACH, hDll = NULL)
	}
	return TRUE;
}
