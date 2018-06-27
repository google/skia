/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WinRTCompat_h
#define WinRTCompat_h

#ifdef SK_BUILD_FOR_WINRT

#include <Windows.h>

#ifdef __cplusplus
#define C_PLUS_PLUS_BEGIN_GUARD    extern "C" {
#define C_PLUS_PLUS_END_GUARD      }
#else
#include <stdbool.h>
#define C_PLUS_PLUS_BEGIN_GUARD
#define C_PLUS_PLUS_END_GUARD
#endif

#include <wchar.h>

C_PLUS_PLUS_BEGIN_GUARD

#ifdef _M_ARM

// This should have been not used, but as the code is designed for x86
// and there is a RUNTIME check for simd, this has to exist. As the
// runtime check will fail, and revert to a C implementation, this is 
// not a problem to have a stub.

// used in: deflate.c
unsigned int _mm_crc32_u32(unsigned int crc, unsigned int v);

#endif // _M_ARM

// used in: dng_string.cpp
int WINAPI CompareStringWCompat(_In_ LCID Locale, _In_ DWORD dwCmpFlags, _In_NLS_string_(cchCount1) PCNZWCH lpString1, _In_ int cchCount1, _In_NLS_string_(cchCount2) PCNZWCH lpString2, _In_ int cchCount2);
// used in: dng_string.cpp
UINT WINAPI GetACPCompat(void);
// used in: dng_pthread.cpp
HANDLE WINAPI OpenThreadCompat(_In_ DWORD dwDesiredAccess, _In_ BOOL bInheritHandle, _In_ DWORD dwThreadId);
// used in: dng_utils.cpp
DWORD  WINAPI GetTickCountCompat(VOID);
// used in: dng_utils.cpp
int WINAPI MessageBoxACompat(_In_opt_ HWND hWnd, _In_opt_ LPCSTR lpText, _In_opt_ LPCSTR lpCaption, _In_ UINT uType);
// used in: dng_pthread.cpp
DWORD WINAPI TlsAllocCompat(VOID);
// used in: dng_pthread.cpp
LPVOID WINAPI TlsGetValueCompat(_In_ DWORD dwTlsIndex);
// used in: dng_pthread.cpp
BOOL WINAPI TlsSetValueCompat(_In_ DWORD dwTlsIndex, _In_opt_ LPVOID lpTlsValue);
// used in: dng_pthread.cpp
BOOL WINAPI TlsFreeCompat(_In_ DWORD dwTlsIndex);
// used in: jsimd_xxx.c
char *getenvCompat(const char *name);

// override any previous declaration with ours

#define MessageBoxA MessageBoxACompat
#define CompareStringW CompareStringWCompat
#define GetACP GetACPCompat
#define OpenThread OpenThreadCompat
#define GetTickCount GetTickCountCompat
#define TlsAlloc TlsAllocCompat
#define TlsGetValue TlsGetValueCompat
#define TlsSetValue TlsSetValueCompat
#define TlsFree TlsFreeCompat
#define getenv getenvCompat

C_PLUS_PLUS_END_GUARD

#endif // SK_BUILD_FOR_WINRT

#endif // WinRTCompat_h
