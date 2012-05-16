
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
#pragma intrinsic(_InterlockedIncrement, _InterlockedDecrement)
#pragma intrinsic(_InterlockedCompareExchange)

int32_t sk_atomic_inc(int32_t* addr) {
    // InterlockedIncrement returns the new value, we want to return the old.
    return _InterlockedIncrement(reinterpret_cast<LONG*>(addr)) - 1;
}

int32_t sk_atomic_dec(int32_t* addr) {
    return _InterlockedDecrement(reinterpret_cast<LONG*>(addr)) + 1;
}
void sk_membar_aquire__after_atomic_dec() { }

int32_t sk_atomic_conditional_inc(int32_t* addr) {
    while (true) {
        LONG value = static_cast<LONG const volatile&>(*addr);
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

