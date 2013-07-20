#include "precompiled.h"
#include "util.h"

#define FILE_SHARE_ALL (FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE)

typedef struct _log_level_prefix {
	wchar_t prefix[8];
	cp_color_t color;
} log_level_prefix_t;

static log_level_prefix_t log_level_prefixes[] = {
	{ L"ERROR", cp_red },
	{ L"WARN", cp_yellow },
	{ L"INFO", cp_white },
	{ L"DEBUG", cp_dark_green },
	{ L"VERBOSE", cp_cyan },
};

void error_message(DWORD dw, wchar_t* message, ...)
{
	void *lpDisplayBuf = NULL, *lpMsgBuf = NULL;

	if(dw == 0) {
		// If no return code was specified, we assume that the message
		// contains a function name that failed. In that case, we retrieve
		// the system error message for the last-error code
		size_t szDisplayLen;
		dw = GetLastError();

		FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (wchar_t*)&lpMsgBuf, 0, NULL
		);

		szDisplayLen = wcslen((const wchar_t*)lpMsgBuf) + wcslen((const wchar_t*)message) + 32;

		// Allocate our buffer for the error message.
		lpDisplayBuf = (void*)LocalAlloc(LPTR, WSIZE(szDisplayLen));
		_snwprintf((wchar_t*)lpDisplayBuf, szDisplayLen,
			L"%s failed with error 0x%08X: %s", message, dw, lpMsgBuf
		);
	} else {
		// Otherwise, we assume that the error message is a format string.
		size_t szDisplayBuf;
		va_list args = NULL;

		// Allocate buffer for our resulting format string.
		va_start(args, message);

		// Check the resulting size of the buffer.
		szDisplayBuf = (size_t)_vscwprintf((const wchar_t*)message, args) + 1;

		// Allocate our buffer.
		lpDisplayBuf = (void*)LocalAlloc(LPTR, WSIZE(szDisplayBuf));

		// Finally, fill in the message.
		_vsnwprintf((wchar_t*)lpDisplayBuf, szDisplayBuf,(const wchar_t*)message, args);
		va_end(args);
	}
	
	XLOGW(LEVEL_ERROR, (const wchar_t*)lpDisplayBuf);
	if(lpMsgBuf) LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}

/** Allocation/Deallocation */
void *xalloc(size_t size)
{
	void *p = malloc(size);
	if (p == NULL) {
		fatalfn(L"malloc");
	}
	memset(p, 0, size);
	return p;
}

void _real_xfree_(void *m)
{
	if (m != NULL) free(m);
}

void _real_xafree_(void **array)
{
	void **ptr = array;
	if (array == NULL) return;
	while (*ptr != NULL)
		xfree(*(ptr++));
	xfree(array);
}

/** String Duplication */
wchar_t *xwcsndup(const wchar_t *s, size_t size)
{
	wchar_t *p;
	size_t szlen;
	if (!s || (size < 0)) return NULL;
	if ((szlen = wcslen(s)) < size)
		size = szlen;
	p = salloc(size, wchar_t);
	memcpy(p, s, size * sizeof(wchar_t));
	return p;
}

int file_exists(const wchar_t* sPath)
{
	DWORD dwAttrib = GetFileAttributesW(sPath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void* file_to_buffer(const wchar_t* path, size_t* size)
{
	DWORD r;
	BYTE* buffer = NULL;
	HANDLE hf;
	LARGE_INTEGER fsize;
	*size = 0;

	// open and read file to buffer
	if((hf = CreateFileW(path, GENERIC_READ, FILE_SHARE_ALL, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE) {
		error_function(L"CreateFile");
	}

	if(!GetFileSizeEx(hf, &fsize)) {
		CloseHandle(hf);
		error_function(L"GetFileSizeEx");
		return buffer;
	}

	if(!(buffer = (BYTE*)calloc(1, fsize.LowPart + 1))) {
		CloseHandle(hf);
		error_function(L"calloc");
		return buffer;
	}

	if (!ReadFile(hf, buffer, fsize.LowPart, &r, NULL)) {
		CloseHandle(hf);
		free(buffer);
		error_function(L"ReadFile");
		buffer = NULL;
		return buffer;
	}

	*size = fsize.LowPart;
	CloseHandle(hf);
	return buffer;
}

/** Output formatting/character attributes/etc */
static HANDLE hStdOut = INVALID_HANDLE_VALUE;
static WORD wOriginalAttrs = 0;

static void setup_colors(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbiCurrent;
	if(hStdOut != INVALID_HANDLE_VALUE) return;
	
	if((hStdOut = GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE) {
		fatalfn(L"GetStdHandle");
	}
	
	if(!GetConsoleScreenBufferInfo(hStdOut, &csbiCurrent)) {
		fatalfn(L"GetConsoleScreenBufferInfo");
	}
	
	wOriginalAttrs = csbiCurrent.wAttributes;
}

int set_color(cp_color_t color)
{
	int result = 1;
	WORD wColor;
	setup_colors();
	wColor = (color == cp_default) ?
		wOriginalAttrs : (WORD)((0 >> 4) | color);

	if(!SetConsoleTextAttribute(hStdOut, wColor)) {
		error_function(L"SetConsoleTextAttribute");
		result = 0;
	}
	return result;
}

int puts_color_a(cp_color_t color, char* sfmt, ...)
{
	int result = 0;
	va_list args = NULL;
	if(!set_color(color)) return -1;
	va_start(args, sfmt);
	vprintf(sfmt, args);
	va_end(args);
	return result;
}

int puts_color_w(cp_color_t color, wchar_t* sfmt, ...)
{
	int result = 0;
	va_list args = NULL;
	if(!set_color(color)) return -1;
	va_start(args, sfmt);
	vwprintf(sfmt, args);
	va_end(args);
	return result;
}

/** Format is [HH:MM:SS] LEVEL: */
#define LOG_PREFIX_FMT L"[%.2d:%.2d:%.2d] %s: "

void generic_log_prefix(log_level_t level)
{
	SYSTEMTIME now = {0};
	log_level_prefix_t* prefix;
	if(level == LEVEL_NOTSET) return;
	prefix = &(log_level_prefixes[(int)level]);
	GetLocalTime(&now);
	print_color_w(prefix->color, LOG_PREFIX_FMT, now.wHour, now.wMinute, now.wSecond, prefix->prefix);
}

// I should probably have a size arg here, but fuck it.
// Paths can exceed MAX_PATH, but for now, we wont support those.
int init_program_info(program_info_t* pinfo)
{
	wchar_t sBuffer[MAX_PATH+1], *sNamePart;
	if(!pinfo) return 0;
	if(!GetModuleFileNameW(NULL, pinfo->path, MAX_PATH) || !(*pinfo->path)) {
		error_function(L"GetModuleFileNameW");
		return 0;
	}
	wcscpy(sBuffer, pinfo->path);
	sNamePart = wcsrchr(sBuffer, L'\\');
	pinfo->name = &(pinfo->path[0]) + (sNamePart - sBuffer + 1); // We don't want the ending '\\''
	*sNamePart = L'\0';
	wcscpy(pinfo->folder, sBuffer);
	return (*pinfo->folder != L'\0') && (*pinfo->name != L'\0');
}

