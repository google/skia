/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_WIN)

#include "include/private/SkMutex.h"
#include "src/core/SkLeanWindows.h"
#include "src/core/SkTLS.h"

#ifdef SK_BUILD_FOR_WINRT

VOID NTAPI onTLSCallback(IN PVOID lpFlsData) {
    if (lpFlsData != nullptr) {
        SkTLS::Destructor(lpFlsData);
    }
}

#define TlsAlloc() FlsAlloc(onTLSCallback);
#define TlsGetValue(dwFlsIndex) FlsGetValue(dwFlsIndex);
#define TlsSetValue(dwFlsIndex, lpFlsData) FlsSetValue(dwFlsIndex, lpFlsData);

#endif//defined SK_BUILD_FOR_WINRT

static bool gOnce = false;
static DWORD gTlsIndex;

void* SkTLS::PlatformGetSpecific(bool forceCreateTheSlot) {
    static SkMutex& mutex = *(new SkMutex);
    if (!forceCreateTheSlot && !gOnce) {
        return nullptr;
    }

    if (!gOnce) {
        SkAutoMutexExclusive tmp(mutex);
        if (!gOnce) {
            gTlsIndex = TlsAlloc();
            gOnce = true;
        }
    }
    return TlsGetValue(gTlsIndex);
}

void SkTLS::PlatformSetSpecific(void* ptr) {
    SkASSERT(gOnce);
    (void)TlsSetValue(gTlsIndex, ptr);
}

#ifndef SK_BUILD_FOR_WINRT

// Call TLS destructors on thread exit. Code based on Chromium's
// base/threading/thread_local_storage_win.cc
#ifdef _WIN64

#pragma comment(linker, "/INCLUDE:_tls_used")
#pragma comment(linker, "/INCLUDE:skia_tls_callback")

#else

#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma comment(linker, "/INCLUDE:_skia_tls_callback")

#endif

void NTAPI onTLSCallback(PVOID unused, DWORD reason, PVOID unused2) {
    if ((DLL_THREAD_DETACH == reason || DLL_PROCESS_DETACH == reason) && gOnce) {
        void* ptr = TlsGetValue(gTlsIndex);
        if (ptr != nullptr) {
            SkTLS::Destructor(ptr);
            TlsSetValue(gTlsIndex, nullptr);
        }
    }
}

extern "C" {

#ifdef _WIN64

#pragma const_seg(".CRT$XLB")
extern const PIMAGE_TLS_CALLBACK skia_tls_callback;
const PIMAGE_TLS_CALLBACK skia_tls_callback = onTLSCallback;
#pragma const_seg()

#else

#pragma data_seg(".CRT$XLB")
PIMAGE_TLS_CALLBACK skia_tls_callback = onTLSCallback;
#pragma data_seg()

#endif
}

#endif//not defined(SK_BUILD_FOR_WINRT)

#endif//defined(SK_BUILD_FOR_WIN)
