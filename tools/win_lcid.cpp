/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "windows.h"
#include "stdio.h"

#define BUFFER_SIZE 512
BOOL CALLBACK MyFuncLocaleEx(LPWSTR pStr, DWORD dwFlags, LPARAM lparam) {
    WCHAR wcBuffer[BUFFER_SIZE];
    int bufferSize;

    bufferSize = GetLocaleInfoEx(pStr, LOCALE_SENGLANGUAGE, wcBuffer, BUFFER_SIZE);
    if (bufferSize == 0) {
        wprintf(L"Locale %s had error %d\n", pStr, GetLastError());
        return (TRUE);
    }

    LCID lcid = LocaleNameToLCID(pStr, nullptr);
    if (lcid == 0) {
        wprintf(L"Error %d getting LCID\n", GetLastError());
        return (TRUE);
    }

    if (lcid > 0x8000) {
        wprintf(L"//");
    }
    wprintf(L"    { 0x%.4x, \"%s\" }, //%s\n", lcid, pStr, wcBuffer);

    return (TRUE);
}

int main(int argc, wchar_t* argv[]) {
    EnumSystemLocalesEx(MyFuncLocaleEx, LOCALE_ALL, nullptr, nullptr);
}
