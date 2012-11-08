/*
util.c - Utility functions.
*/

#include "precompiled.h"

TCHAR	prog_path[MAX_PATH];
LPTSTR	prog;
int	log_level = 3 + 4 + 8 + 16;
char	tempfile[MAX_PATH];
DWORD	pid;


LPCTSTR get_filename_ext(LPCTSTR filename) {
	LPCTSTR dot = _tcsrchr(filename, _T('.'));
	if(!dot || dot == filename) return EMPTY_TSTR;
	return dot + 1;
}

// Get just the name of the program: "C:\path\program.exe" -> "program".
// Returns a pointer within program; it is modified to remove the extension.
LPTSTR get_program_name( LPTSTR program )
{
	LPTSTR name, ext;

	if (program == NULL)
	{
		GetModuleFileName( NULL, prog_path, lenof(prog_path) );
		program = prog_path;
	}
	name = wcsrchr( program, '\\' );
	if (name != NULL)
	++name;
	else
	name = program;
	ext = wcsrchr( name, '.' );
	if (ext != NULL && ext != name)
	*ext = '\0';

	return name;
}


void DEBUGSTR( int level, LPTSTR szFormat, ... )
{
	TCHAR szBuffer[1024], szEscape[1024];
	va_list pArgList;
	HANDLE mutex;
	DWORD wait;
	FILE* file;

	if ((log_level & 3) < level && !(level & 4 & log_level))
	return;

	if (*tempfile == '\0')
	{
		_snprintf( tempfile, MAX_PATH, "%s\\procrewriter.log", getenv( "TEMP" ) );
		pid = GetCurrentProcessId();
	}
	if (szFormat == NULL)
	{
		file = fopen( tempfile, "wt" );
		if (file != NULL)
		{
			SYSTEMTIME now;
			GetLocalTime( &now );
			fprintf( file, "ProcessRewriter v" PR_VERSA " log (%d) started "
			"%d-%.2d-%.2d %d:%.2d:%.2d\n",
			log_level,
			now.wYear, now.wMonth, now.wDay,
			now.wHour, now.wMinute, now.wSecond );
			fclose( file );
		}
		return;
	}

	va_start( pArgList, szFormat );
	_vsnwprintf( szBuffer, lenof(szBuffer), szFormat, pArgList );
	va_end( pArgList );

	szFormat = szBuffer;
	if (*szFormat == '\33')
	{
		BOOL first = TRUE;
		LPTSTR pos = szEscape;
		while (*++szFormat != '\0' && pos < szEscape + lenof(szEscape) - 4)
		{
			if (*szFormat < 32)
			{
				*pos++ = '\\';
				switch (*szFormat)
				{
				case '\a': *pos++ = 'a'; break;
				case '\b': *pos++ = 'b'; break;
				case '\t': *pos++ = 't'; break;
				case '\r': *pos++ = 'r'; break;
				case '\n': *pos++ = 'n'; break;
					case	27 : *pos++ = 'e'; break;
				default:
					pos += _snwprintf( pos, 32, L"%.*o",
					(szFormat[1] >= '0' && szFormat[1] <= '7') ? 3 : 1,
					*szFormat );
				}
			}
			else
			{
				if (*szFormat == '"')
				{
					if (first)
					first = FALSE;
					else if (szFormat[1] != '\0')
					*pos++ = '\\';
				}
				*pos++ = *szFormat;
			}
		}
		*pos = '\0';
		szFormat = szEscape;
	}

	mutex = CreateMutex( NULL, FALSE, L"procrewriter_debug_file" );
	wait	= WaitForSingleObject( mutex, 500 );
	file	= fopen( tempfile, "at" ); // _fmode might be binary
	if (file != NULL)
	{
		fwprintf( file, L"%s (%lu): %s\n", prog, pid, szFormat );
		fclose( file );
	}
	if (wait == WAIT_OBJECT_0)
	ReleaseMutex( mutex );
	CloseHandle( mutex );
}

/**
* Malloc that causes process exit in case of ENOMEM
*/
void *xmalloc(size_t size)
{
	void *p = calloc(size, 1);
	if (p == 0) {
		DEBUGSTR(1, (LPTSTR)_T("FATAL: Error during xmalloc - Size: %lu\n"), size);
		_exit(1);
	}
	return p;
}

void xfree(void *m)
{
	if (m != 0)
		free(m);
}

static inline size_t get_file_length(FILE* pFile)
{
	long lCurrPos, lEndPos;
	size_t result = -1;
	lCurrPos = ftell(pFile);
	if(lCurrPos == -1)
		return result;
	if(fseek(pFile, 0L, SEEK_END) == -1)
		return result;
	lEndPos = ftell(pFile);
	if(lEndPos == -1)
		return result;
	result = (size_t)(lEndPos - lCurrPos);
	if(fseek(pFile, 0L, SEEK_SET) == -1)
		return -1;
	return result;
}

BOOL file_to_buffer (LPTSTR sPath, PBYTE* buffer, size_t* size)
{
	FILE* fp;
	fp = _tfopen(sPath, _T("rb"));
	if (fp == NULL) {
		DEBUGSTR(1, (LPTSTR)_T("Could not read file: %s\n"), sPath);
		return FALSE;
	}
	*size = get_file_length(fp);
	if(*size == -1) {
		DEBUGSTR(1, (LPTSTR)_T("Could not get the size of file: %s!\n"), sPath);
		return FALSE;
	}
	*buffer = (PBYTE)xmalloc(*size);
	if(!*buffer) {
		DEBUGSTR(1, (LPTSTR)_T("Could allocate our buffer for the contents of file: %s!\n"), sPath);
		return FALSE;
	}
	ZeroMemory(*buffer, *size);
	fread(*buffer, 1, *size, fp);
	fclose(fp);
	return TRUE;
}



/* String Array Stuff */

char **straalloc(size_t size)
{
	return (char **)xmalloc((size + 1) * sizeof(char *));
}

wchar_t **waalloc(size_t size)
{
	return (wchar_t **)xmalloc((size + 1) * sizeof(wchar_t *));
}

void strafree(char **array)
{
	char **ptr = array;

	if (array == 0)
		return;
	while (*ptr != 0)
		xfree(*(ptr++));
	xfree(array);
}

void wafree(wchar_t **array)
{
	wchar_t **ptr = array;

	if (array == 0)
		return;
	while (*ptr != 0)
		xfree(*(ptr++));
	xfree(array);
}

/* String duplication stuff */

char *xstrdup(const char *s)
{
	char *d;
	if (s == 0)
		return 0;
	d = _strdup(s);
	if (d == 0) {
		DEBUGSTR(1, (LPTSTR)_T("FATAL: Error during xstrdup\n"));
		_exit(1);
	}
	return d;
}

wchar_t *xwcsdup(const wchar_t *s)
{
	wchar_t *d;
	if (s == 0)
		return 0;
	d = _wcsdup(s);
	if (d == 0) {
		_wperror(L"wcsdup");
		_exit(1);
	}
	return d;
}

char *xstrndup(const char *s, size_t size)
{
	char *p;

	if (s == 0)
		return 0;
	if (strlen(s) < size)
		size = strlen(s);
	p = (char *)xmalloc((size + 2) * sizeof(wchar_t));
	memcpy(p, s, size * sizeof(char));
	return p;
}

wchar_t *xwcsndup(const wchar_t *s, size_t size)
{
	wchar_t *p;

	if (s == 0)
		return 0;
	if (wcslen(s) < size)
		size = wcslen(s);
	p = (wchar_t *)xmalloc((size + 2) * sizeof(wchar_t));
	memcpy(p, s, size * sizeof(wchar_t));
	return p;
}
