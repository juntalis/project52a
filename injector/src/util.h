/**
 * @file util.h
 * @brief Commonly used utility functions.
 *
 * This header assumes precompiled.h has already been included in the current
 * context.
 */
 
#ifndef _UTIL_H_
#define _UTIL_H_
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/** Fatal handlers. */
void abort_execution(DWORD);
void error_message(DWORD, wchar_t*, ...);
#define error_function(func) error_message(ERROR_SUCCESS, func)
#define fatalme() fatal(0L, PP_WIDEN(__FUNCTION__))
#define fatalfn(f) fatal(0L, f)
#define fatal(r,m,...) { \
		error_message(r, m, __VA_ARGS__); \
		abort_execution(r); \
	} 


/** Utility allocation function */
void* xalloc(size_t);
#define typalloc(T) ((T*)xalloc(sizeof(T)))
#define aalloc(X,T) ((T*)xalloc((X) * sizeof(T)))
#define salloc(X,T) aalloc((X+1),T)
#define wcsalloc(X) salloc(X,wchar_t)
#define stralloc(X) salloc(X,char)

/** Utility memory deallocation function */
void _real_xfree_(void*);
void _real_xafree_(void**);
#define xfree(X) _real_xfree_((void*)(X))
#define xafree(X) _real_xafree_((void**)(X))

/** Generic string handling */
wchar_t *xwcsndup(const wchar_t *, size_t);

/** Filepaths */
int file_exists(const wchar_t*);
void* file_to_buffer(const wchar_t*, size_t* );

typedef enum {
	cp_default = -1,
	cp_black = 0,
	cp_dark_blue = 1,
	cp_dark_green = 2,
	cp_dark_cyan = 3,
	cp_dark_red = 4,
	cp_dark_magenta = 5,
	cp_dark_yellow  = 6,
	cp_gray = 7,
	cp_dark_gray = 8,
	cp_blue = 9,
	cp_green = 10,
	cp_cyan = 11,
	cp_red = 12,
	cp_magenta = 13,
	cp_yellow = 14,
	cp_white = 15
} cp_color_t;

int set_color(cp_color_t);
#define reset_color() set_color(cp_default)

// The following two functions do not reset the color afterwards.
int puts_color_a(cp_color_t, char*, ...);
int puts_color_w(cp_color_t, wchar_t*, ...);

#define print_color_a(C,...) { \
	puts_color_a(C, __VA_ARGS__); \
	reset_color(); \
}

#define print_color_w(C,...) { \
	puts_color_w(C, __VA_ARGS__); \
	reset_color(); \
}

typedef enum {
	LEVEL_NOTSET = -1,
	LEVEL_ERROR = 0,
	LEVEL_WARN,
	LEVEL_INFO,
	LEVEL_DEBUG,
	LEVEL_VERBOSE
} log_level_t;

void generic_log_prefix(log_level_t);

#define XLOGW(LVL, ...) { \
	generic_log_prefix(LVL); \
	wprintf(__VA_ARGS__); \
	wprintf(L"\n"); \
}

#define XLOGA(LVL, ...) { \
	generic_log_prefix(LVL); \
	printf(__VA_ARGS__); \
	printf("\n"); \
}

#define xinfo(...) XLOGW(LEVEL_INFO, __VA_ARGS__)
#define xdebug(...) XLOGW(LEVEL_DEBUG, __VA_ARGS__)
#define xverbose(...) XLOGW(LEVEL_VERBOSE, __VA_ARGS__)
#define xwarn(...) XLOGW(LEVEL_WARN, __VA_ARGS__)

typedef struct {
	wchar_t* name; // Points to the character just after the last \\ in path.
	wchar_t path[MAX_PATH+1];
	wchar_t folder[MAX_PATH+1];
} program_info_t;

// Expected to be called by the program at startup.
int init_program_info(program_info_t*);

#define ptr_to_dw(ptr) ((DWORD)(DWORD_PTR)(ptr))

#ifdef __cplusplus
}
#endif

#endif /* _UTIL_H_ */
