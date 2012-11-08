#include "precompiled.h"
#include "injector.h"

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
BOOL find_parent_proc(LPPROCESS_INFORMATION ppi)
{
	HANDLE hSnap;
	PROCESSENTRY32 pe;
	THREADENTRY32	te;
	DWORD id = GetCurrentProcessId();
	BOOL fOk;

	hSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS|TH32CS_SNAPTHREAD, id );

	if (hSnap == INVALID_HANDLE_VALUE)
		return FALSE;

	find_proc_id( hSnap, id, &pe );
	if (!find_proc_id( hSnap, pe.th32ParentProcessID, &pe ))
	{
		CloseHandle( hSnap );
		return FALSE;
	}

	te.dwSize = sizeof(te);
	for (fOk = Thread32First( hSnap, &te ); fOk; fOk = Thread32Next( hSnap, &te ))
		if (te.th32OwnerProcessID == pe.th32ProcessID)
			break;

	CloseHandle( hSnap );

	ppi->dwProcessId = pe.th32ProcessID;
	ppi->dwThreadId	= te.th32ThreadID;

	return fOk;
}
