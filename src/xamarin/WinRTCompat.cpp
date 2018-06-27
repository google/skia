/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "WinRTCompat.h"

#ifdef SK_BUILD_FOR_WINRT

#include <windows.h>

#ifdef _M_ARM

unsigned int _mm_crc32_u32(unsigned int crc, unsigned int v)
{
	return 0;
}

#endif // _M_ARM

DWORD WINAPI TlsAllocCompat(VOID)
{
	return ::FlsAlloc(NULL);
}

LPVOID WINAPI TlsGetValueCompat(_In_ DWORD dwTlsIndex)
{
	return ::FlsGetValue(dwTlsIndex);
}

BOOL WINAPI TlsSetValueCompat(_In_ DWORD dwTlsIndex, _In_opt_ LPVOID lpTlsValue)
{
	return ::FlsSetValue(dwTlsIndex, lpTlsValue);
}

BOOL WINAPI TlsFreeCompat(_In_ DWORD dwTlsIndex)
{
	return ::FlsFree(dwTlsIndex);
}

char *getenvCompat(const char *name)
{
	// there is no environment in WinRT
	return NULL;
}

int WINAPI CompareStringWCompat(_In_ LCID Locale, _In_ DWORD dwCmpFlags, _In_NLS_string_(cchCount1) PCNZWCH lpString1, _In_ int cchCount1, _In_NLS_string_(cchCount2) PCNZWCH lpString2, _In_ int cchCount2)
{
	WCHAR localeName[LOCALE_NAME_MAX_LENGTH];
	LCIDToLocaleName(Locale, localeName, LOCALE_NAME_MAX_LENGTH, 0);
	return CompareStringEx(localeName, dwCmpFlags, lpString1, cchCount1, lpString2, cchCount2, NULL, NULL, NULL);
}

UINT WINAPI GetACPCompat(void)
{
	return CP_ACP;
}

HANDLE WINAPI OpenThreadCompat(_In_ DWORD dwDesiredAccess, _In_ BOOL bInheritHandle, _In_ DWORD dwThreadId)
{
	// TODO: implementation
	return NULL;
}

DWORD WINAPI GetTickCountCompat(VOID)
{
	return (long)GetTickCount64();
}

int WINAPI MessageBoxACompat(_In_opt_ HWND hWnd, _In_opt_ LPCSTR lpText, _In_opt_ LPCSTR lpCaption, _In_ UINT uType)
{
	// TODO: implementation
	return 0;
}

#endif // SK_BUILD_FOR_WINRT
