/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMutex_pthread_DEFINED
#define SkMutex_pthread_DEFINED

/** Posix pthread_mutex based mutex. */

#include <errno.h>
#include <pthread.h>

// We use error-checking mutexes in Debug builds or normal fast mutexes in Release builds.
// Debug builds get these checks for free:
//   - a double acquire() from the same thread fails immediately instead of deadlocking;
//   - release() checks that the mutex is being unlocked by its owner thread.
// I don't see a built-in way to implement assertHeld(), so we track that with an fOwner field.

// This isn't technically portable, but on Linux and Android pthread_t is some sort of int, and
// on Darwin it's a pointer.  So assuming pthread_self() never returns 0, it works as a sentinel.
SkDEBUGCODE(static const pthread_t kNoOwner = 0;)

// An SkBaseMutex is a POD structure that can be directly initialized at declaration time with
// SK_DECLARE_STATIC_MUTEX. This avoids the generation of a static initializer in the final
// machine code (and a corresponding static finalizer).
struct SkBaseMutex {
    void acquire() {
        SkDEBUGCODE(int rc = ) pthread_mutex_lock(&fMutex);
        SkASSERT(0 == rc);
        SkDEBUGCODE(fOwner = pthread_self();)
    }
    void release() {
        this->assertHeld();  // Usually redundant, but not for static mutexes on Macs (see below).
        SkDEBUGCODE(fOwner = kNoOwner;)
        SkDEBUGCODE(int rc = ) pthread_mutex_unlock(&fMutex);
        SkASSERT(0 == rc);
    }
    void assertHeld() {
        SkASSERT(0 != pthread_equal(fOwner, pthread_self()));
    }

    pthread_mutex_t fMutex;
    SkDEBUGCODE(pthread_t fOwner;)  // Read and write only when holding fMutex.
};

// A normal mutex that's required to be initialized through normal C++ construction,
// i.e. when it's a member of another class, or allocated on the heap.
class SkMutex : public SkBaseMutex {
public:
    SkMutex() {
#ifdef SK_DEBUG
        pthread_mutexattr_t attr;
        SkASSERT(0 == pthread_mutexattr_init(&attr));
        SkASSERT(0 == pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK));
        SkASSERT(0 == pthread_mutex_init(&fMutex, &attr));
        SkASSERT(0 == pthread_mutexattr_destroy(&attr));
        fOwner = kNoOwner;
#else
        (void)pthread_mutex_init(&fMutex, NULL);
#endif
    }

    ~SkMutex() {
        SkDEBUGCODE(int rc = )pthread_mutex_destroy(&fMutex);
        SkASSERT(0 == rc);
    }

private:
    SkMutex(const SkMutex&);
    SkMutex& operator=(const SkMutex&);
};

#if defined(SK_DEBUG) && defined(PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP)
    // When possible we want to use error-check mutexes in Debug builds.  See the note at the top.
    #define SK_BASE_MUTEX_INIT { PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP, kNoOwner }
#elif defined(SK_DEBUG)
    // Macs don't support PTHREAD_ERRORCHECK_MUTEX_INITIALIZER when targeting <10.7. We target 10.6.
    #define SK_BASE_MUTEX_INIT { PTHREAD_MUTEX_INITIALIZER, kNoOwner }
#else
    #define SK_BASE_MUTEX_INIT { PTHREAD_MUTEX_INITIALIZER }
#endif

// Using POD-style initialization prevents the generation of a static initializer.
//
// Without magic statics there are no thread safety guarantees on initialization
// of local statics (even POD). As a result, it is illegal to use
// SK_DECLARE_STATIC_MUTEX in a function.
//
// Because SkBaseMutex is not a primitive, a static SkBaseMutex cannot be
// initialized in a class with this macro.
#define SK_DECLARE_STATIC_MUTEX(name) namespace {} static SkBaseMutex name = SK_BASE_MUTEX_INIT

#endif
