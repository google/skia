
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include <windows.h>
#include <intrin.h>
#include "SkThread.h"
#include "SkTLS.h"

//MSDN says in order to declare an interlocked function for use as an
//intrinsic, include intrin.h and put the function in a #pragma intrinsic
//directive.
//The pragma appears to be unnecessary, but doesn't hurt.
#pragma intrinsic(_InterlockedIncrement, _InterlockedExchangeAdd, _InterlockedDecrement)
#pragma intrinsic(_InterlockedCompareExchange)

int32_t sk_atomic_inc(int32_t* addr) {
    // InterlockedIncrement returns the new value, we want to return the old.
    return _InterlockedIncrement(reinterpret_cast<LONG*>(addr)) - 1;
}

int32_t sk_atomic_add(int32_t* addr, int32_t inc) {
    return _InterlockedExchangeAdd(reinterpret_cast<LONG*>(addr),
                                   static_cast<LONG>(inc));
}

int32_t sk_atomic_dec(int32_t* addr) {
    return _InterlockedDecrement(reinterpret_cast<LONG*>(addr)) + 1;
}
void sk_membar_aquire__after_atomic_dec() { }

int32_t sk_atomic_conditional_inc(int32_t* addr) {
    while (true) {
        LONG value = static_cast<int32_t const volatile&>(*addr);
        if (value == 0) {
            return 0;
        }
        if (_InterlockedCompareExchange(reinterpret_cast<LONG*>(addr),
                                        value + 1,
                                        value) == value) {
            return value;
        }
    }
}
void sk_membar_aquire__after_atomic_conditional_inc() { }

SkMutex::SkMutex() {
    SK_COMPILE_ASSERT(sizeof(fStorage) > sizeof(CRITICAL_SECTION),
                      NotEnoughSizeForCriticalSection);
    InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&fStorage));
}

SkMutex::~SkMutex() {
    DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&fStorage));
}

void SkMutex::acquire() {
    EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&fStorage));
}

void SkMutex::release() {
    LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&fStorage));
}

///////////////////////////////////////////////////////////////////////////

static bool gOnce;
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
