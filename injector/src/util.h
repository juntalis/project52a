/*
util.h - Utility functions.
*/

#ifndef UTIL_H
#define UTIL_H
#pragma once

LPTSTR get_program_name( LPTSTR program );
void DEBUGSTR( int level, LPTSTR szFormat, ... );
BOOL file_to_buffer (LPTSTR sPath, PBYTE* buffer, size_t* size);
LPCTSTR get_filename_ext(LPCTSTR filename);

void *xmalloc(size_t size);
void xfree(void *m);

char **straalloc(size_t size);
wchar_t **waalloc(size_t size);
void strafree(char **array);
void wafree(wchar_t **array);

char *xstrdup(const char *s);
wchar_t *xwcsdup(const wchar_t *s);
char *xstrndup(const char *s, size_t size);
wchar_t *xwcsndup(const wchar_t *s, size_t size);


#endif /* UTIL_H */