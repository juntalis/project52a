/**
 * @file injdll32_64.c
 * @brief Functions for extracting the export offsets in x86 DLLs. (For use from x86)
 */

#include "precompiled.h"
#include "util.h"
#include "injector.h"

DWORD find_export_x86(wchar_t* sDllName, char* sTarget)
{
	HMODULE hMod;
	DWORD dwResult = 0;
	FARPROC pExport = NULL;
	if(!(hMod = LoadLibraryW(sDllName))) {
		error_function(L"LoadLibraryW");
		return dwResult;
	}

	if(!(pExport = GetProcAddress(hMod, sTarget))) {
		error_function(L"GetProcAddress");
		FreeLibrary(hMod);
		return dwResult;
	}

	dwResult = (DWORD)(ptr_to_dw(pExport) - ptr_to_dw(hMod));
	FreeLibrary(hMod);
	return dwResult;
}
