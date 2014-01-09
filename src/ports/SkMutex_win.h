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
// we can't easily get rid of static initializers.
class SkMutex {
public:
    SkMutex() {
        InitializeCriticalSection(&fStorage);
    }

    ~SkMutex() {
        DeleteCriticalSection(&fStorage);
    }

    void acquire() {
        EnterCriticalSection(&fStorage);
    }

    void release() {
        LeaveCriticalSection(&fStorage);
    }

private:
    SkMutex(const SkMutex&);
    SkMutex& operator=(const SkMutex&);

    CRITICAL_SECTION fStorage;
};

typedef SkMutex SkBaseMutex;

// Windows currently provides no documented means of POD initializing a CRITICAL_SECTION.
#define SK_DECLARE_STATIC_MUTEX(name) static SkBaseMutex name
#define SK_DECLARE_GLOBAL_MUTEX(name) SkBaseMutex name

#endif
