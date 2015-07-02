/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTLS.h"
#include "SkThread.h"

static bool gOnce = false;
static DWORD gTlsIndex;
SK_DECLARE_STATIC_MUTEX(gMutex);

void* SkTLS::PlatformGetSpecific(bool forceCreateTheSlot) {
    if (!forceCreateTheSlot && !gOnce) {
        return NULL;
    }

    if (!gOnce) {
        SkAutoMutexAcquire tmp(gMutex);
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
        if (ptr != NULL) {
            SkTLS::Destructor(ptr);
            TlsSetValue(gTlsIndex, NULL);
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
