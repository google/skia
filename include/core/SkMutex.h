/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMutex_DEFINED
#define SkMutex_DEFINED

// This file is not part of the public Skia API.
#include "../private/SkSemaphore.h"
#include "SkTypes.h"

#define SK_MUTEX_SEMAPHORE_INIT {1, {0}}

#ifdef SK_DEBUG
    #define SK_BASE_MUTEX_INIT {SK_MUTEX_SEMAPHORE_INIT, 0}
#else
    #define SK_BASE_MUTEX_INIT {SK_MUTEX_SEMAPHORE_INIT}
#endif

// Using POD-style initialization prevents the generation of a static initializer.
//
// Without magic statics there are no thread safety guarantees on initialization
// of local statics (even POD). As a result, it is illegal to use
// SK_DECLARE_STATIC_MUTEX in a function.
//
// Because SkBaseMutex is not a primitive, a static SkBaseMutex cannot be
// initialized in a class with this macro.
#define SK_DECLARE_STATIC_MUTEX(name) namespace {} static SkBaseMutex name = SK_BASE_MUTEX_INIT;

// TODO(herb): unify this with the ThreadID in SkSharedMutex.cpp.
#ifdef SK_DEBUG
    #ifdef SK_BUILD_FOR_WIN
        #include <windows.h>
        inline int64_t sk_get_thread_id() { return GetCurrentThreadId(); }
    #else
        #include <pthread.h>
        inline int64_t sk_get_thread_id() { return (int64_t)pthread_self(); }
    #endif
#endif

typedef int64_t SkThreadID;

SkDEBUGCODE(static const SkThreadID kNoOwner = 0;)

struct SkBaseMutex {
    void acquire() {
        fSemaphore.wait();
        SkDEBUGCODE(fOwner = sk_get_thread_id();)
    }

    void release() {
        this->assertHeld();
        SkDEBUGCODE(fOwner = kNoOwner;)
        fSemaphore.signal();
    }

    void assertHeld() {
        SkASSERT(fOwner == sk_get_thread_id());
    }

    SkBaseSemaphore fSemaphore;
    SkDEBUGCODE(SkThreadID fOwner;)
};

// This needs to use subclassing instead of encapsulation to make SkAutoMutexAcquire to work.
class SkMutex : public SkBaseMutex {
public:
    SkMutex () {
        fSemaphore = SK_MUTEX_SEMAPHORE_INIT;
        SkDEBUGCODE(fOwner = kNoOwner);
    }
    ~SkMutex () { fSemaphore.deleteSemaphore(); }
    SkMutex(const SkMutex&) = delete;
    SkMutex& operator=(const SkMutex&) = delete;
};

template <typename Lock>
class SkAutoTAcquire : SkNoncopyable {
public:
    explicit SkAutoTAcquire(Lock& mutex) : fMutex(&mutex) {
        SkASSERT(fMutex != NULL);
        mutex.acquire();
    }

    explicit SkAutoTAcquire(Lock* mutex) : fMutex(mutex) {
        if (mutex) {
            mutex->acquire();
        }
    }

    /** If the mutex has not been released, release it now. */
    ~SkAutoTAcquire() {
        if (fMutex) {
            fMutex->release();
        }
    }

    /** If the mutex has not been released, release it now. */
    void release() {
        if (fMutex) {
            fMutex->release();
            fMutex = NULL;
        }
    }

    /** Assert that we're holding the mutex. */
    void assertHeld() {
        SkASSERT(fMutex);
        fMutex->assertHeld();
    }

private:
    Lock* fMutex;
};

typedef SkAutoTAcquire<SkBaseMutex> SkAutoMutexAcquire;
#define SkAutoMutexAcquire(...) SK_REQUIRE_LOCAL_VAR(SkAutoMutexAcquire)

#endif//SkMutex_DEFINED
