
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkThread.h"

#include <pthread.h>
#include <errno.h>

/**
 We prefer the GCC intrinsic implementation of the atomic operations over the
 SkMutex-based implementation. The SkMutex version suffers from static 
 destructor ordering problems.
 Note clang also defines the GCC version macros and implements the intrinsics.
 TODO: Verify that gcc-style __sync_* intrinsics work on ARM
 According to this the intrinsics are supported on ARM in LLVM 2.7+
 http://llvm.org/releases/2.7/docs/ReleaseNotes.html
*/
#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 1) || __GNUC__ > 4
    #if (defined(__x86_64) || defined(__i386__))
        #define GCC_INTRINSIC
    #endif
#endif

#if defined(GCC_INTRINSIC)

int32_t sk_atomic_inc(int32_t* addr)
{
    return __sync_fetch_and_add(addr, 1);
}

int32_t sk_atomic_dec(int32_t* addr)
{
    return __sync_fetch_and_add(addr, -1);
}

#else

SkMutex gAtomicMutex;

int32_t sk_atomic_inc(int32_t* addr)
{
    SkAutoMutexAcquire ac(gAtomicMutex);

    int32_t value = *addr;
    *addr = value + 1;
    return value;
}

int32_t sk_atomic_dec(int32_t* addr)
{
    SkAutoMutexAcquire ac(gAtomicMutex);
    
    int32_t value = *addr;
    *addr = value - 1;
    return value;
}

#endif

//////////////////////////////////////////////////////////////////////////////

static void print_pthread_error(int status)
{
    switch (status) {
    case 0: // success
        break;
    case EINVAL:
        printf("pthread error [%d] EINVAL\n", status);
        break;
    case EBUSY:
        printf("pthread error [%d] EBUSY\n", status);
        break;
    default:
        printf("pthread error [%d] unknown\n", status);
        break;
    }
}

SkMutex::SkMutex(bool isGlobal) : fIsGlobal(isGlobal)
{
    if (sizeof(pthread_mutex_t) > sizeof(fStorage))
    {
        SkDEBUGF(("pthread mutex size = %d\n", sizeof(pthread_mutex_t)));
        SkASSERT(!"mutex storage is too small");
    }

    int status;
    pthread_mutexattr_t attr;

    status = pthread_mutexattr_init(&attr);
    print_pthread_error(status);
    SkASSERT(0 == status);
    
    status = pthread_mutex_init((pthread_mutex_t*)fStorage, &attr);
    print_pthread_error(status);
    SkASSERT(0 == status);
}

SkMutex::~SkMutex()
{
    int status = pthread_mutex_destroy((pthread_mutex_t*)fStorage);
    
    // only report errors on non-global mutexes
    if (!fIsGlobal)
    {
        print_pthread_error(status);
        SkASSERT(0 == status);
    }
}

void SkMutex::acquire()
{
    int status = pthread_mutex_lock((pthread_mutex_t*)fStorage);
    print_pthread_error(status);
    SkASSERT(0 == status);
}

void SkMutex::release()
{
    int status = pthread_mutex_unlock((pthread_mutex_t*)fStorage);
    print_pthread_error(status);
    SkASSERT(0 == status);
}

