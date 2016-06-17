/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMutex_DEFINED
#define SkMutex_DEFINED

#include "../private/SkSemaphore.h"
#include "../private/SkThreadID.h"
#include "SkTypes.h"

#define SK_DECLARE_STATIC_MUTEX(name) static SkBaseMutex name;

class SkBaseMutex {
public:
    constexpr SkBaseMutex() = default;

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

protected:
    SkBaseSemaphore fSemaphore{1};
    SkDEBUGCODE(SkThreadID fOwner{kIllegalThreadID};)
};

class SkMutex : public SkBaseMutex {
public:
    using SkBaseMutex::SkBaseMutex;
    ~SkMutex() { fSemaphore.cleanup(); }
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
