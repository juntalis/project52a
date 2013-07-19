// precompiled.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Configuration & Setup
#include <project52a/macros/preconfig.h>
#include <project52a/build/common.h>
#include <project52a/version.h>

// System Headers
#include <windows.h>
#include <winnt.h>
#include <tlhelp32.h>
#include <tchar.h>

#if defined(BUILD_LANG_C)
#	include <stdlib.h>
#	include <stdio.h>
#	include <string.h>
#	include <stdarg.h>
#	include <errno.h>
#	include <assert.h>
#elif defined(BUILD_LANG_CPP)
#	include <cstdlib>
#	include <cstdio>
#	include <cstring>
#	include <cstdarg>
#	include <cerrno>
#	include <cassert>
#endif

// Disable warnings.
#pragma warning(disable:4996)

// MSVC's C compiler doesn't support the inline
// keyword.
#ifndef BUILD_LANG_CPP
//	To be safe..
#	ifdef inline
#		undef inline
#	endif
#	define inline __forceinline
#else
extern "C" {
#endif

#ifdef BUILD_ARCH_X86
	typedef unsigned __int32 ptr_t;
#else
	typedef unsigned __int64 ptr_t;
#endif

// Not even sure how much I'll be using the declarations below.
typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

typedef signed __int8  s8;
typedef signed __int16 s16;
typedef signed __int32 s32;
typedef signed __int64 s64;

typedef u8  byte;
typedef u16 word;
typedef u32 dword;
typedef u64 qword;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#ifdef BUILD_LANG_CPP
};
#endif
