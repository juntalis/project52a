#pragma once

/* Windows Version */
// - Target Windows XP and higher.
// - Throw a compile-time error if a lower platform is detected.
#if !defined(WINVER)
#	define WINVER 0x0501
#elif (WINVER < 0x0501)
#	error Windows XP is currently the lowest version of Windows supported by this project.
#endif

#if !defined(_WIN32_WINNT)
#	define _WIN32_WINNT 0x0501
#elif (_WIN32_WINNT < 0x0501)
#	error Windows XP is currently the lowest version of Windows supported by this project.
#endif

#if !defined(NTDDI_VERSION)
#	define NTDDI_VERSION 0x05010000
#elif (NTDDI_VERSION < 0x05010000)
#	error Windows XP is currently the lowest version of Windows supported by this project.
#endif

/* MSVC-Specific Definitions */
// Speed up build process
#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif
#ifndef VC_EXTRALEAN
#	define VC_EXTRALEAN
#endif

// Warning Suppression
#ifndef _CRT_SECURE_NO_WARNINGS
#	define _CRT_SECURE_NO_WARNINGS
#endif

// CRT API Declarations
#ifdef _MT
#	ifdef _DLL
#		define CRT_SHARED
#		define MSVCRT_API __declspec(dllimport)
#	else
		PP_WARN("Because Command Prompt already uses the shared version of the CRT library (msvcrt.dll), it is suggested that you use the same.")
#		define CRT_STATIC
#		define MSVCRT_API 
#	endif
#else
#	error This project requires multithreading support!
#endif

// Character-set
// - TODO: Support for ASCII strings by predefining USE_ASCII_STR, but warn on compilation.
// - If no character-set is defined, and USE_ASCII_STR is not defined default to unicode.
// - Define MBCS/UNICODE depending on the target character-set.
#if defined(USE_ASCII_STR)
#	if !defined(_UNICODE) && !defined(_MBCS)
#		define BUILD_STR_ASCII
		PP_WARN("The use of an ASCII character set for strings is an afterthought. It is therefore highly recommended that you use multibyte or unicode strings instead.")
#		error Not currently implemented. (TODO)
#	else
#		error _UNICODE & _MBCS cannot be defined if USE_ASCII_STR is specified!
#	endif
#elif defined(_UNICODE)
#	define BUILD_STR_UNICODE
#elif defined(_MBCS)
#	define BUILD_STR_MBCS
#else
#	define _UNICODE
#	define BUILD_STR_UNICODE
#endif

// Architecture Detection
#ifndef _WIN32
#	define _WIN32
#endif
#if defined(_M_IX86) && !defined(_M_IA64) && !defined(_M_AMD64) && !defined(_M_X64)
#	define BUILD_ARCH_X86
//	Check for SSE/SSE2 extensions
#	if defined(_M_IX86_FP)
#		if _M_IX86_FP
#			define BUILD_WITH_SSE 1
#		endif
#		if _M_IX86_FP > 1
#			define BUILD_WITH_SSE2 1
#		endif
#	endif
#elif defined(_M_AMD64)
#	define X64_ARCH_AMD64
#	define BUILD_ARCH_X64
#	ifndef _WIN64
#		define _WIN64
#	endif
#elif defined(_M_IA64)
#	define X64_ARCH_IA64
#	define BUILD_ARCH_X64
#	ifndef _WIN64
#		define _WIN64
#	endif
#elif defined(_M_X64)
#	define BUILD_ARCH_X64
#	ifndef _WIN64
#		define _WIN64
#	endif
#else
#	error Could not detect platform architecture.
#endif

#include <SDKDDKVer.h>
