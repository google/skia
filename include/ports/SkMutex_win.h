/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMutex_win_DEFINED
#define SkMutex_win_DEFINED

/** Windows CriticalSection based mutex. */

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#  define WIN32_IS_MEAN_WAS_LOCALLY_DEFINED
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#  define NOMINMAX_WAS_LOCALLY_DEFINED
#endif
#
#include <windows.h>
#
#ifdef WIN32_IS_MEAN_WAS_LOCALLY_DEFINED
#  undef WIN32_IS_MEAN_WAS_LOCALLY_DEFINED
#  undef WIN32_LEAN_AND_MEAN
#endif
#ifdef NOMINMAX_WAS_LOCALLY_DEFINED
#  undef NOMINMAX_WAS_LOCALLY_DEFINED
#  undef NOMINMAX
#endif

// On Windows, SkBaseMutex and SkMutex are the same thing,
// we can't easily get rid of static initializers. However,
// we preserve the same inheritance pattern as other platforms
// so that we can forward-declare cleanly.
struct SkBaseMutex {
public:
    SkBaseMutex() {
        InitializeCriticalSection(&fStorage);
        SkDEBUGCODE(fOwner = 0;)
    }

    ~SkBaseMutex() {
        SkASSERT(0 == fOwner);
        DeleteCriticalSection(&fStorage);
    }

    void acquire() {
        EnterCriticalSection(&fStorage);
        SkDEBUGCODE(fOwner = GetCurrentThreadId();)
    }

    void release() {
        this->assertHeld();
        SkDEBUGCODE(fOwner = 0;)
        LeaveCriticalSection(&fStorage);
    }

    void assertHeld() {
        SkASSERT(GetCurrentThreadId() == fOwner);
    }

protected:
    CRITICAL_SECTION fStorage;
    SkDEBUGCODE(DWORD fOwner;)

private:
    SkBaseMutex(const SkBaseMutex&);
    SkBaseMutex& operator=(const SkBaseMutex&);
};

class SkMutex : public SkBaseMutex { };

// Windows currently provides no documented means of POD initializing a CRITICAL_SECTION.
// As a result, it is illegal to SK_DECLARE_STATIC_MUTEX in a function.
#define SK_DECLARE_STATIC_MUTEX(name) namespace{} static SkBaseMutex name

#endif
