
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include <windows.h>
#include "SkThread.h"

int32_t sk_atomic_inc(int32_t* addr)
{
    // InterlockedIncrement returns the new value, we want to return the old.
    return InterlockedIncrement(reinterpret_cast<LONG*>(addr)) - 1;
}

int32_t sk_atomic_dec(int32_t* addr)
{
    return InterlockedDecrement(reinterpret_cast<LONG*>(addr)) + 1;
}

SkMutex::SkMutex(bool /* isGlobal */)
{
    SK_COMPILE_ASSERT(sizeof(fStorage) > sizeof(CRITICAL_SECTION),
                      NotEnoughSizeForCriticalSection);
    InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&fStorage));
}

SkMutex::~SkMutex()
{
    DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&fStorage));
}

void SkMutex::acquire()
{
    EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&fStorage));
}

void SkMutex::release()
{
    LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&fStorage));
}

