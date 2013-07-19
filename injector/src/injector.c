#include "precompiled.h"
#include "util.h"
#include "injector.h"

static program_info_t program_info = { NULL, { 0 }, { 0 } };

void abort_execution(DWORD code)
{
	ExitProcess(code);
}

// Find the name of the DLL and inject it.
int inject(LPPROCESS_INFORMATION ppi, wchar_t* sProcName, BOOL* gui)
{
	wchar_t* sDllPath = NULL;
	size_t szPath;
	int iProcArch;

	xdebug(L"%s (%lu)", program_info.name, ppi->dwProcessId );
	iProcArch = process_type(ppi, gui);

	if (iProcArch == 0) {
		error_message(1, L"Unsupported process: %s", sProcName);
		return 0;
	}

	szPath = wcslen(program_info.folder) + sizeof(PROJ_DLL) + 1;
	sDllPath = wcsalloc(szPath);
#ifdef BUILD_ARCH_X64
	if (iProcArch == 32) {
		_snwprintf(sDllPath, szPath, L"%s\\%s", program_info.folder, PROJ_X86_DLL);
		inject_x86(ppi, sDllPath, szPath);
	} else {
		_snwprintf(sDllPath, szPath, L"%s\\%s", program_info.folder, PROJ_X64_DLL);
		inject_x64(ppi, sDllPath);
	}
#else
	_snwprintf(sDllPath, szPath, L"%s\\%s", program_info.folder, PROJ_X86_DLL);
	inject_x86(ppi, sDllPath, szPath);
#endif
	xfree(sDllPath);
	return 1;
}

int get_debug_privileges(HANDLE hProc) 
{
	int result = 0;
	HANDLE hToken;
	LUID seDebugNameValue;
	TOKEN_PRIVILEGES tp;

	if (!OpenProcessToken(hProc,TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken )) {
		error_function(L"OpenProcessToken");
		return result;
	}

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &seDebugNameValue )) {
		CloseHandle(hToken);
		error_function(L"LookupPrivilegeValue");
		return result;
	}

	tp.PrivilegeCount=1;
	tp.Privileges[0].Luid = seDebugNameValue;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if(!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL )) {
		error_function(L"AdjustTokenPrivileges");
	} else {
		result = 1;
	}

	CloseHandle(hToken);
	return result;
} 

int wmain(int argc, TCHAR* argv[])
{
	wchar_t sProcName[MAX_PATH+1] = L"",
			sDllPath[MAX_PATH+1] = L"";
	HANDLE hProc;
	PROCESS_INFORMATION pi;
	BOOL gui = FALSE;
	int rc = 0;
	
	if(!init_program_info(&program_info)) {
		fatal(1, L"Could not get executable path information.");
	}

	hProc = GetCurrentProcess();
	xdebug(L"Acquiring debug privileges..");
	if(!get_debug_privileges(hProc)) {
		return 1;
	}
	
	if(!find_parent_proc(&pi, sProcName)) {
		fatal(1, L"Could not get parent information.");
	}

	pi.hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pi.dwProcessId);
	pi.hThread  = OpenThread(THREAD_ALL_ACCESS, FALSE, pi.dwThreadId );
	SuspendThread(pi.hThread);
	
	if (!inject( &pi, sProcName, &gui )) {
		rc = 1;
	}

	ResumeThread( pi.hThread );
	CloseHandle( pi.hThread );
	CloseHandle( pi.hProcess );
	return rc;
}
