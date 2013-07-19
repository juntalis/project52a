#pragma once

/* User Config */
#ifdef WITH_CONFIG
#	include "config.h"
#endif

/* Compiler-based Macros */
// Include compiler-specific macros
#if defined(_MSC_VER)
#	include <project52a/build/msvc.h>
#else
//	TODO: Write a buildconfig.mingw.h header. (Maybe)
#	error Could not detect compiler!
#endif

// C/C++ Language Detection
#ifdef __cplusplus
#	define BUILD_LANG_CPP
#else
#	define BUILD_LANG_C
#endif

// Config/Release Configuration
#if (defined(_DEBUG) || defined(DEBUG)) && !defined(_NDEBUG) && !defined(NDEBUG)
#	define BUILD_CFG_DEBUG
#	ifndef _DEBUG
#		define _DEBUG
#	endif
//	 We're going to be using this for something else.
#	ifdef DEBUG
#		undef DEBUG
#	endif
#elif (defined(_NDEBUG) || defined(NDEBUG)) && !defined(_DEBUG) && !defined(DEBUG)
#	define BUILD_CFG_RELEASE
#	ifndef _NDEBUG
#		define _NDEBUG
#	endif
#elif (defined(_DEBUG) || defined(DEBUG)) && (defined(_NDEBUG) || defined(NDEBUG))
#	error Cannot define _DEBUG and _NDEBUG at the same time!
#else
#	define _NDEBUG
#	define BUILD_CFG_RELEASE
#endif

// Project-Specific Macros

/**
 * WITH_PROC_OFFSETS - Allow hard-coding 32-bit function offsets to speed up the injection process.
 * This will result in an injector that may not work correctly on other versions of Windows/pycomspec
 * DLLs compiled under different configurations. Defining this flag causes us to assume that 
 * proc_offsets.c exists, as well.
 * 
 * TODO: Write a script to generate proc_offsets.c
 */
#ifdef WITH_PROC_OFFSETS
#	define PROJECT52A_STATIC_PROC32_OFFSETS
#endif

/**
 * WITH_CMD_PATCHES - Target only a specific version of Command Prompt. This will cause pycomspec to
 * immediately begin patching with a specific patch set. While this should speed up the patching process,
 * it will also break your version of pycomspec for other versions of Windows/Command Prompt. This also
 * causes us to assume that cmd_patches.c exists.
 * 
 * TODO: Write a script to generate cmd_patches.c and implement this.
 */
#ifdef WITH_CMD_PATCHES
#	define PROJECT52A_STATIC_PATCHES
#endif
