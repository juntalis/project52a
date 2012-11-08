#pragma once

// Target Windows XP & higher
#ifdef WINVER
#	undef WINVER
#endif
#define WINVER 0x0500

#ifdef _WIN32_WINNT
#	undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0501

#ifdef NTDDI_VERSION
#	undef NTDDI_VERSION
#endif
#define NTDDI_VERSION 0x05010000

// Make sure that we're using a unicode character set.
#ifndef UNICODE
#	define UNICODE
#endif

// Speed up build process with minimal headers.
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#include <SDKDDKVer.h>
