/**
 * injector.h
 */

#ifndef INJECTOR_H
#define INJECTOR_H
#pragma once

#include "util.h"

void InjectDLL32( LPPROCESS_INFORMATION, LPCTSTR );

#ifdef _WIN64
void InjectDLL64( LPPROCESS_INFORMATION, LPCTSTR );
#endif

BOOL find_parent_proc(LPPROCESS_INFORMATION ppi);
int ProcessType( LPPROCESS_INFORMATION pinfo, BOOL* gui );

extern TCHAR  prog_path[MAX_PATH];
extern LPTSTR prog;
extern int	log_level;

#endif /* INJECTOR_H */