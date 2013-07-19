#include "precompiled.h"
#include "injector.h"
#include "util.h"

// Search each process in the snapshot for id.
static BOOL find_proc_id(HANDLE snap, DWORD id, LPPROCESSENTRY32 ppe)
{
	BOOL fOk;
	ppe->dwSize = sizeof(PROCESSENTRY32);
	for (fOk = Process32First( snap, ppe ); fOk; fOk = Process32Next( snap, ppe ))
		if (ppe->th32ProcessID == id)
			break;
	return fOk;
}

// Obtain the process and thread identifiers of the parent process.
int find_parent_proc(LPPROCESS_INFORMATION ppi, wchar_t* sProcName)
{
	HANDLE hSnap;
	PROCESSENTRY32 pe;
	THREADENTRY32 te;
	BOOL fOk;
	int id = GetCurrentProcessId();
	hSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS|TH32CS_SNAPTHREAD, id );

	if (hSnap == INVALID_HANDLE_VALUE)
		return 0;

	find_proc_id( hSnap, id, &pe );
	if (!find_proc_id( hSnap, pe.th32ParentProcessID, &pe )) {
		CloseHandle( hSnap );
		return 0;
	}

	te.dwSize = sizeof(te);
	for (fOk = Thread32First( hSnap, &te ); fOk; fOk = Thread32Next(hSnap, &te))
		if (te.th32OwnerProcessID == pe.th32ProcessID)
			break;

	CloseHandle(hSnap);

	ppi->dwProcessId = pe.th32ProcessID;
	ppi->dwThreadId	= te.th32ThreadID;
	wcscpy(sProcName, pe.szExeFile);
	return fOk;
}

int process_type(LPPROCESS_INFORMATION pinfo, BOOL* gui)
{
	char* ptr;
	MEMORY_BASIC_INFORMATION minfo;
	IMAGE_DOS_HEADER dos_header;
	IMAGE_NT_HEADERS nt_header;
	SIZE_T read;

	*gui = FALSE;
	for (ptr = NULL;
		VirtualQueryEx( pinfo->hProcess, ptr, &minfo, sizeof(minfo) );
		ptr += minfo.RegionSize)
	{
		if (minfo.BaseAddress == minfo.AllocationBase &&
			ReadProcessMemory( pinfo->hProcess, minfo.AllocationBase,
			&dos_header, sizeof(dos_header), &read ))
		{
			if (dos_header.e_magic == IMAGE_DOS_SIGNATURE)
			{
				if (ReadProcessMemory( pinfo->hProcess, (char*)minfo.AllocationBase +
					dos_header.e_lfanew, &nt_header,
					sizeof(nt_header), &read ))
				{
					if (nt_header.Signature == IMAGE_NT_SIGNATURE &&
						(nt_header.FileHeader.Characteristics &
						(IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_DLL))
						== IMAGE_FILE_EXECUTABLE_IMAGE)
					{
						*gui = (nt_header.OptionalHeader.Subsystem
							== IMAGE_SUBSYSTEM_WINDOWS_GUI);
						if (nt_header.OptionalHeader.Subsystem ==
							IMAGE_SUBSYSTEM_WINDOWS_CUI || *gui)
						{
							if (nt_header.FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
							{
								// Microsoft ignores precision on %p.
								xdebug(L"  32-bit %s (base = %.8X)",
									(*gui) ? L"GUI" : L"console",
									(DWORD)(DWORD_PTR)minfo.AllocationBase );
								return 32;
							}
#ifdef BUILD_ARCH_X64
							if (nt_header.FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
							{
								xdebug(L"  64-bit %s (base = %p)",
									(*gui) ? L"GUI" : L"console", minfo.AllocationBase );
								return 64;
							}
#else
							if (nt_header.FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
							{
								xdebug(L"  64-bit %s",
									(*gui) ? L"GUI" : L"console" );
								return 64;
							}
#endif
							xdebug(L"  Ignoring unsupported machine (0x%X)",
								nt_header.FileHeader.Machine );
						}
						else
						{
							xdebug(L"  Ignoring unsupported subsystem (%u)",
								nt_header.OptionalHeader.Subsystem );
						}
						return 0;
					}
				}
			}
		}

		// If a 32-bit process manages to load a 64-bit one, we may miss the base
		// address.  If the pointer overflows, assume 64-bit and abort.
		if (ptr > ptr + minfo.RegionSize) {
#ifdef BUILD_ARCH_X64
			xdebug(L"  Pointer overflow: assuming 64-bit console" );
			return 64;
#else
			xdebug(L"  Ignoring apparent 64-bit process" );
			return 0;
#endif
		}
	}

	xdebug(L"  Ignoring non-Windows process" );
	return 0;
}

// I'd prefer to go the route below, but to do so, I need to find a portable way of locating
// a process's main thread that won't get messed up with architecture mismatches.
#if 0

#ifndef PROCESS_QUERY_LIMITED_INFORMATION
#	define PROCESS_QUERY_LIMITED_INFORMATION 0x1000
#endif

LONG (WINAPI *NtQueryInformationProcess)(
	HANDLE ProcessHandle,
	ULONG ProcessInformationClass,
	PVOID ProcessInformation,
	ULONG ProcessInformationLength,
	PULONG ReturnLength
); 

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
	ULONG AllocationSize;
	ULONG Size;
	ULONG Flags;
	ULONG DebugFlags;
	HANDLE hConsole;
	ULONG ProcessGroup;
	HANDLE hStdInput;
	HANDLE hStdOutput;
	HANDLE hStdError;
	UNICODE_STRING CurrentDirectoryName;
	HANDLE CurrentDirectoryHandle;
	UNICODE_STRING DllPath;
	UNICODE_STRING ImagePathName;
	UNICODE_STRING CommandLine;
	PWSTR Environment;
	ULONG dwX;
	ULONG dwY;
	ULONG dwXSize;
	ULONG dwYSize;
	ULONG dwXCountChars;
	ULONG dwYCountChars;
	ULONG dwFillAttribute;
	ULONG dwFlags;
	ULONG wShowWindow;
	UNICODE_STRING WindowTitle;
	UNICODE_STRING DesktopInfo;
	UNICODE_STRING ShellInfo;
	UNICODE_STRING RuntimeInfo;
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

typedef void* PPEB_LDR_DATA;
typedef void* PPS_POST_PROCESS_INIT_ROUTINE;

typedef struct _PEB {
	BYTE Reserved1[2];
	BYTE BeingDebugged;
	BYTE Reserved2[1];
	PVOID Reserved3[2];
	PPEB_LDR_DATA Ldr;
	PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
	BYTE Reserved4[104];
	PVOID Reserved5[52];
	PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
	BYTE Reserved6[128];
	PVOID Reserved7[1];
	ULONG SessionId;
} PEB, *PPEB;

struct PROCESS_BASIC_INFORMATION {
	PVOID Reserved1;
	PPEB PebBaseAddress; 
	PVOID Reserved2[2];
	ULONG_PTR UniqueProcessId;
	PVOID Reserved3;
};

DWORD find_parent_proc(void)
{
	HANDLE hProc;
	PROCESS_BASIC_INFORMATION pbi;
	ULONG ulSize = 0;
	hProc = GetCurrentProcess();
	*((FARPROC*)(&NtQueryInformationProcess)) = GetProcAddress(GetModuleHandleA("NTDLL.DLL"), "NtQueryInformationProcess");
	if (NtQueryInformationProcess) {
		LONG ret = NtQueryInformationProcess(hProc, 0, &pbi, sizeof(pbi), &ulSize);
		if ((ret >= 0) && (size == sizeof(ulSize))) {
			return (DWORD)pbi.Reserved3;
		}
	}
	return -1;
}

#endif

