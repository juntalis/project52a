/**
 * @file injector.h
 * @brief Common header for the injector project.
 */

#ifndef INJECTOR_H
#define INJECTOR_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int find_parent_proc(LPPROCESS_INFORMATION, wchar_t*);
int process_type(LPPROCESS_INFORMATION, BOOL*);

void inject_x86(LPPROCESS_INFORMATION, wchar_t*, size_t);
DWORD find_export_x86(wchar_t*, char*);

#ifdef BUILD_ARCH_X64
void inject_x64(LPPROCESS_INFORMATION, wchar_t*);
#endif

#ifdef __cplusplus
}
#endif

#endif /* INJECTOR_H */
