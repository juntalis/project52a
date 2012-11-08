/**
 * procrewriter.h - Common macros and declarations.
 */

#ifndef PROCREWRITER_H
#define PROCREWRITER_H

#pragma once

#define lenof(array) (sizeof(array)/sizeof(*(array)))
#define TSIZE(size)  ((size) * sizeof(TCHAR))

#define PRX86_DLL _TEXT("procrewriter.dll")
#define PRX64_DLL _TEXT("procrewriter.x64.dll")

#ifdef _WIN64
#	define PR_DLL PRX64_DLL
#	define POINTER_TYPE ULONGLONG
#else
#	define PR_DLL PRX86_DLL
#	define POINTER_TYPE DWORD
#endif

#define PR_VERS	L"1.00"         // wide string
#define PR_VERSA	 "1.00"         // ANSI string (windres 2.16.91 didn't like L)
#define PR_VERE	L"100"          // wide environment string
#define PR_VEREA	 "100"          // ANSI environment string
#define PR_VERB	1,0,0,0 	// binary (resource)

// For intellisense.
#if defined(_WIN64) && !defined(_AMD64_) && !defined(_IA64_)
#	define _AMD64_
#endif

// Just some utility macros and definitions
#define EMPTY_TSTR _TEXT("")

#ifndef inline
#	define inline __forceinline
#endif

#endif
