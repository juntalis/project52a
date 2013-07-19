//{{NO_DEPENDENCIES}}
#pragma once

#ifdef _DEBUG
#	define PROJ_DLL_BASE _TEXT("project52ad")
#else
#	define PROJ_DLL_BASE _TEXT("project52a")
#endif

#define PROJ_X86_DLL PROJ_DLL_BASE _TEXT("-32.dll")
#define PROJ_X64_DLL PROJ_DLL_BASE _TEXT("-64.dll")

#define PROJ_VERS L"1.0.0.1"
#define PROJ_VERSA "1.0.0.1"
#define PROJ_VERE L"1001"
#define PROJ_VEREA "1001"
#define PROJ_VERB 1,0,0,1

#define PROJ_AUTHOR "Charles Grunwald (Juntalis) <ch@rles.grunwald.me>"
#ifdef _WIN64
#	define PROJ_DLL PROJ_X64_DLL
#	define POINTER_TYPE ULONGLONG
#else
#	define PROJ_DLL PROJ_X86_DLL
#	define POINTER_TYPE DWORD
#endif
