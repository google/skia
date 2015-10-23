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

#ifdef SK_DEBUG
    #include "../private/SkThreadID.h"
#endif

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

struct SkBaseMutex {
    void acquire() {
        fSemaphore.wait();
        SkDEBUGCODE(fOwner = SkGetThreadID();)
    }

    void release() {
        this->assertHeld();
        SkDEBUGCODE(fOwner = kIllegalThreadID;)
        fSemaphore.signal();
    }

    void assertHeld() {
        SkASSERT(fOwner == SkGetThreadID());
    }

    SkBaseSemaphore fSemaphore;
    SkDEBUGCODE(SkThreadID fOwner;)
};

// This needs to use subclassing instead of encapsulation to make SkAutoMutexAcquire to work.
class SkMutex : public SkBaseMutex {
public:
    SkMutex () {
        fSemaphore = SK_MUTEX_SEMAPHORE_INIT;
        SkDEBUGCODE(fOwner = kIllegalThreadID);
    }
    ~SkMutex () { fSemaphore.deleteSemaphore(); }
    SkMutex(const SkMutex&) = delete;
    SkMutex& operator=(const SkMutex&) = delete;
};

template <typename Lock>
class SkAutoTAcquire : SkNoncopyable {
public:
    explicit SkAutoTAcquire(Lock& mutex) : fMutex(&mutex) {
        SkASSERT(fMutex != nullptr);
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
            fMutex = nullptr;
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

// SkAutoTExclusive is a lighter weight version of SkAutoTAcquire. It assumes that there is a valid
// mutex, thus removing the check for the null pointer.
template <typename Lock>
class SkAutoTExclusive {
public:
    SkAutoTExclusive(Lock& lock) : fLock(lock) { lock.acquire(); }
    ~SkAutoTExclusive() { fLock.release(); }
private:
    Lock &fLock;
};

typedef SkAutoTAcquire<SkBaseMutex> SkAutoMutexAcquire;
#define SkAutoMutexAcquire(...) SK_REQUIRE_LOCAL_VAR(SkAutoMutexAcquire)

typedef SkAutoTExclusive<SkBaseMutex> SkAutoMutexExclusive;
#define SkAutoMutexExclusive(...) SK_REQUIRE_LOCAL_VAR(SkAutoMutexExclusive)

#endif//SkMutex_DEFINED
