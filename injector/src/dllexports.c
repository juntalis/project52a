/**
 * dllexports.c - Functions for extracting the names and offsets of a DLL's export, regardless of architecture.
 */

#include "precompiled.h"
#include "util.h"
#include "dllexports.h"

#ifdef _WIN64
#	define GetX86SysDir GetSystemWow64Directory
#else
#	define GetX86SysDir GetSystemDirectory
#endif

#define MAKE_PTR( cast, ptr, addValue ) (cast)( (DWORD_PTR)(ptr) + (DWORD_PTR)(addValue))
#define GET_HEADER_DICTIONARY(headers, idx) (&(headers->OptionalHeader.DataDirectory[idx]))
#define GET_DIRECTORY_RVA(headers, idx) (GET_HEADER_DICTIONARY(headers, idx)->VirtualAddress)
#define GET_DIRECTORY_SIZE(headers, idx) (GET_HEADER_DICTIONARY(headers, idx)->Size)

static DllExport empty_export = { NULL, -1 };

static PIMAGE_SECTION_HEADER get_enclosing_header_x86(DWORD rva, PIMAGE_NT_HEADERS32 nt_headers)
{
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_headers);
	unsigned i;

	for ( i=0; i < nt_headers->FileHeader.NumberOfSections; i++, section++ )
	{
		DWORD size = section->Misc.VirtualSize;
		if ( 0 == size )
			size = section->SizeOfRawData;

		if ( (rva >= section->VirtualAddress) && (rva < (section->VirtualAddress + size)))
			return section;
	}

	return 0;
}

static LPVOID get_ptr_from_rva_x86( DWORD rva, PIMAGE_NT_HEADERS32 pNTHeader, PBYTE imageBase )
{
	PIMAGE_SECTION_HEADER pSectionHdr;
	INT delta;

	pSectionHdr = get_enclosing_header_x86( rva, pNTHeader );
	if ( !pSectionHdr )
		return 0;

	delta = (INT)(pSectionHdr->VirtualAddress-pSectionHdr->PointerToRawData);
	return (PVOID) ( imageBase + rva - delta );
}

static void add_x86_export(DWORD i, PDllExport* lpExports, const char* funcName, DWORD funcOffset)
{
	PDllExport pNewExport = &((*lpExports)[i]);
	pNewExport->name = xstrdup(funcName);
	pNewExport->offset = funcOffset;
}

static BOOL enumerate_exports_x86(PDllExport* lpExports, PBYTE codeBase, PIMAGE_NT_HEADERS32 nt_headers, BOOL noname)
{
	PIMAGE_EXPORT_DIRECTORY exports;
	PIMAGE_SECTION_HEADER header;
	DWORD i, *nameRef;
	WORD *ordinal;
	//PDWORD funcbase = NULL; 
	DWORD startRVA, endRVA;

	startRVA = GET_DIRECTORY_RVA(nt_headers, IMAGE_DIRECTORY_ENTRY_EXPORT);
	endRVA = startRVA + GET_DIRECTORY_SIZE(nt_headers, IMAGE_DIRECTORY_ENTRY_EXPORT);
	
	header = get_enclosing_header_x86( startRVA, nt_headers );
	if ( !header ) return FALSE;

	exports = (PIMAGE_EXPORT_DIRECTORY)get_ptr_from_rva_x86(startRVA, nt_headers, codeBase);
	if (exports->NumberOfNames == 0 || exports->NumberOfFunctions == 0) return TRUE;
	//funcbase =	(PDWORD)get_ptr_from_rva_x86( exports->AddressOfFunctions, nt_headers, imgBase );
	nameRef = (PDWORD) (codeBase + exports->AddressOfNames);
	ordinal = (PWORD) (codeBase + exports->AddressOfNameOrdinals);
	if (!exports || /*!funcbase ||*/ !ordinal || !nameRef) return FALSE;

	*lpExports = (PDllExport)xmalloc(sizeof(DllExport) * (exports->NumberOfNames + 1));
	for (i = 0; i < exports->NumberOfNames; i++, nameRef++, ordinal++) {
		DWORD funcOffset;
		const char* name = (const char *) (codeBase + (*nameRef));
		funcOffset = ((DWORD)codeBase + (*(DWORD *) (codeBase + exports->AddressOfFunctions + ((*ordinal)*4))));
		add_x86_export(i, lpExports, name, funcOffset);
	}
	(*lpExports)[i] = empty_export;
	return TRUE;
}

BOOL dump_exports(PDllExport* lpExports, LPCTSTR dllName, BOOL blIsX64)
{
	size_t fsize = 0;
	TCHAR dllPath[MAX_PATH + 1] = EMPTY_TSTR;
	PBYTE buffer = NULL;
	PIMAGE_DOS_HEADER dos_header;
	PIMAGE_NT_HEADERS nt_headers;
	BOOL result = FALSE;
	*lpExports = NULL;

	if(_taccess(dllName, 00) == -1) {
		TCHAR dllFilename[MAX_PATH] = EMPTY_TSTR;
		_tcscpy(dllFilename, dllName);

		if(_tcsicmp(get_filename_ext(dllFilename), _T("dll")) != 0) {
			_tcscat(dllFilename, _T(".dll"));
		}

		if(!blIsX64) {
			if(!GetX86SysDir((LPTSTR)dllPath, MAX_PATH)) {
				DEBUGSTR(1, (LPTSTR)_T("Failed to get X86 system directory!\n"));
				return FALSE;
			}
			_tcscat(dllPath, _T("\\"));
			_tcscat(dllPath, dllFilename);
		} else {
			_tsearchenv(dllFilename, _T("PATH"), dllPath);
			if(_tcslen(dllPath) == 0) {
				DEBUGSTR(1, (LPTSTR)_T("Failed to find DLL: %s!\n"), dllName);
				return FALSE;
			}
		}

		if(!_taccess(dllName, 00)) {
			DEBUGSTR(1, (LPTSTR)_T("Could not find DLL: %s! (Checked: %s)\n"), dllName, dllPath);
			return FALSE;
		}

	} else {
		_tcscpy(dllPath, dllName);
	}

	if(!file_to_buffer(dllPath, &buffer, &fsize)) {
		DEBUGSTR(1, (LPTSTR)_T("Could not read DLL: %s!\n"), dllPath);
		return FALSE;
	}

	dos_header = (PIMAGE_DOS_HEADER)buffer;
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
		DEBUGSTR(1, (LPTSTR)_T("Not a valid executable file.\n"));
		goto cleanup;
	}

	nt_headers = MAKE_PTR( PIMAGE_NT_HEADERS, dos_header, dos_header->e_lfanew );
	if (nt_headers->Signature != IMAGE_NT_SIGNATURE) {
		DEBUGSTR(1, _T("No PE header found.\n"));
		goto cleanup;
	}
	
	if(nt_headers->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC && !blIsX64) {
		DEBUGSTR(1, _T("IsX86 flagged as true, but x64 DLL found at: %s.\n"), dllPath);
		goto cleanup;
	} else if(nt_headers->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC && blIsX64) {
		/*if(!enumerate_exports_x64(buffer, MAKE_PTR( PIMAGE_NT_HEADERS64, dos_header, dos_header->e_lfanew ), TRUE)) {
			DEBUGSTR(1, _T("Failed to enumerate exports of %s.\n"), dllPath);
			goto cleanup;
		} */
		// We don't really need this, to be honest.
		DEBUGSTR(1, _T("Somehow you ended up in the x64 part of the dump_exports function.\n"));
		goto cleanup;
	} else {
		if(!enumerate_exports_x86(lpExports, buffer, MAKE_PTR( PIMAGE_NT_HEADERS32, dos_header, dos_header->e_lfanew ), TRUE)) {
			DEBUGSTR(1, _T("Failed to enumerate exports of %s.\n"), dllPath);
			goto cleanup;
		}
	}
	result = TRUE;
cleanup:
	free(buffer);
	return result;
}

DWORD find_export(PDllExport lpExports, const char* name)
{
	int i = 0;
	PDllExport pNewExport = &(lpExports[i]);
	while(&pNewExport != NULL && pNewExport->offset != -1) {
		if(_stricmp(name, pNewExport->name) == 0) {
			return pNewExport->offset;
		}
		pNewExport = &(lpExports[++i]);
	}
	return -1;
}

/*
static PIMAGE_SECTION_HEADER get_enclosing_header_x64(DWORD rva, PIMAGE_NT_HEADERS64 nt_headers)
{
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_headers);
	unsigned i;

	for ( i=0; i < nt_headers->FileHeader.NumberOfSections; i++, section++ )
	{
		DWORD size = section->Misc.VirtualSize;
		if ( 0 == size )
			size = section->SizeOfRawData;

		if ( (rva >= section->VirtualAddress) && (rva < (section->VirtualAddress + size)))
			return section;
	}

	return 0;
}

static LPVOID get_ptr_from_rva_x64( DWORD rva, PIMAGE_NT_HEADERS64 nt_headers, PBYTE imageBase )
{
	PIMAGE_SECTION_HEADER pSectionHdr;
	INT delta;

	pSectionHdr = get_enclosing_header_x64( rva, nt_headers );
	if ( !pSectionHdr )
		return 0;

	delta = (INT)(pSectionHdr->VirtualAddress-pSectionHdr->PointerToRawData);
	return (PVOID) ( imageBase + rva - delta );
}

static void add_x64_export(PDllExport64* lpExports, const char* funcName, ULONGLONG funcOffset)
{
	PDllExport32 pNewExport = (PDllExport64)xmalloc(sizeof(DllExport32));
	pNewExport->name = xstrdup(funcName);
	pNewExport->offset = funcOffset;
	HASH_ADD_STR(*lpExports, name, pNewExport);
}

static BOOL enumerate_exports_x64(PDllExport32* lpExports, PBYTE codeBase, PIMAGE_NT_HEADERS64 nt_headers, BOOL noname)
{
	PIMAGE_EXPORT_DIRECTORY exports;
	PIMAGE_SECTION_HEADER header;
	DWORD i, *nameRef;
	WORD *ordinal;
	//PDWORD funcbase = NULL; 
	DWORD startRVA, endRVA;

	startRVA = GET_DIRECTORY_RVA(nt_headers, IMAGE_DIRECTORY_ENTRY_EXPORT);
	endRVA = startRVA + GET_DIRECTORY_SIZE(nt_headers, IMAGE_DIRECTORY_ENTRY_EXPORT);

	header = get_enclosing_header_x64( startRVA, nt_headers );
	if ( !header ) return FALSE;

	exports = (PIMAGE_EXPORT_DIRECTORY)get_ptr_from_rva_x64(startRVA, nt_headers, codeBase);
	if (exports->NumberOfNames == 0 || exports->NumberOfFunctions == 0) return TRUE;
	//funcbase =	(PDWORD)get_ptr_from_rva_x64( exports->AddressOfFunctions, nt_headers, imgBase );
	nameRef = (PDWORD) (codeBase + exports->AddressOfNames);
	ordinal = (PWORD) (codeBase + exports->AddressOfNameOrdinals);
	if (!exports || !ordinal || !nameRef) return FALSE;

	for (i = 0; i < exports->NumberOfNames; i++, nameRef++, ordinal++) {
		ULONGLONG funcOffset;
		const char* name = (const char *) (codeBase + (*nameRef));
		funcOffset = (*(ULONGLONG *) (codeBase + exports->AddressOfFunctions + ((*ordinal)*4)));
		add_x64_export(lpExports, name, funcOffset);
	}

	return TRUE;
}
*/
