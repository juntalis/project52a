/**
 * procrewriter.h - Common macros and declarations.
 */

#ifndef PROCREWRITER_H
#define PROCREWRITER_H

#pragma once

// Just some utility macros and definitions
#define EMPTY_TSTR _TEXT("")
#define lenof(array) (sizeof(array)/sizeof(*(array)))
#define TSIZE(size)  ((size) * sizeof(TCHAR))

#ifndef inline
#	define inline __forceinline
#endif

#endif
