#include "precompiled.h"
#include "injector.h"

// Find the name of the DLL and inject it.
BOOL Inject( LPPROCESS_INFORMATION ppi, BOOL* gui, LPCTSTR app )
{
	DWORD len;
	WCHAR dll[MAX_PATH];
	int	type;

	DEBUGSTR( 1, L"%s (%lu)", app, ppi->dwProcessId );
	type = ProcessType( ppi, gui );
	if (type == 0)
	{
		fwprintf( stderr, L"ANSICON: %s: unsupported process.\n", app );
		return FALSE;
	}

	len = (DWORD)(prog - prog_path);
	memcpy( dll, prog_path, TSIZE(len) );
#ifdef _WIN64
	
	if (type == 32) {
		wcscpy( dll + len, PRX86_DLL );
		InjectDLL32( ppi, dll );
	} else {
		wcscpy( dll + len, PRX64_DLL );
		InjectDLL64( ppi, dll );
	}
#else
	wcscpy( dll + len, L"ANSI32.dll" );
	InjectDLL32( ppi, dll );
#endif
	return TRUE;
}


int _tmain(int argc, TCHAR* argv[])
{
	PROCESS_INFORMATION pi;
	BOOL gui = FALSE;
	int rc = 0;
	if(!find_parent_proc(&pi)) {
		_tprintf(_TEXT("Fatal: Could not get parent information.\n"));
		return 1;
	}
	prog = get_program_name( NULL );

	pi.hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pi.dwProcessId);
	pi.hThread  = OpenThread( THREAD_ALL_ACCESS,	FALSE, pi.dwThreadId );
	SuspendThread( pi.hThread );
	_getch();
	if (!Inject( &pi, &gui, argv[0] ))
		rc = 1;
	ResumeThread( pi.hThread );
	CloseHandle( pi.hThread );
	CloseHandle( pi.hProcess );
	_getch();
	return rc;
}