/**
 * dllexports.h - Functions for extracting the names and offsets of a DLL's export, regardless of architecture.
 */

#ifndef DLLEXPORTS_H
#define DLLEXPORTS_H
#pragma once

struct DllExport32 {
	char* name;
	DWORD offset;
};
typedef struct DllExport32* PDllExport32;

#define DllExport struct DllExport32
#define PDllExport PDllExport32

/*
struct DllExport64 {
	char* name;
	ULONGLONG offset;
	UT_hash_handle hh;
};
typedef struct DllExport64* PDllExport64;
*/


BOOL dump_exports(PDllExport* lpExports, LPCTSTR dllName, BOOL blIsX64);
DWORD find_export(PDllExport pExports, const char* name);



#endif /* DLLEXPORTS_H */