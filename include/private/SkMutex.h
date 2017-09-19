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
#include "SkTSAN.h"
#include "SkTypes.h"

#define SK_DECLARE_STATIC_MUTEX(name) static SkBaseMutex name;

// We use ANNOTE_RWLOCK_foo() macros in here because TSAN does
// not intercept Mach semaphore_t calls, and so on Mac can't
// see that SkMutex is indeed a mutex.

class SkBaseMutex {
public:
    constexpr SkBaseMutex() = default;

    void acquire() {
        fSemaphore.wait();
        SkDEBUGCODE(fOwner = SkGetThreadID();)
        ANNOTATE_RWLOCK_ACQUIRED(&fSemaphore, true);
    }

    void release() {
        this->assertHeld();
        ANNOTATE_RWLOCK_RELEASED(&fSemaphore, true);
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
    SkMutex() { ANNOTATE_RWLOCK_CREATE(&fSemaphore); }
    ~SkMutex() {
        ANNOTATE_RWLOCK_DESTROY(&fSemaphore);
        fSemaphore.cleanup();
    }
};

class SkAutoMutexAcquire {
public:
    template <typename T>
    SkAutoMutexAcquire(T* mutex) : fMutex(mutex) {
        if (mutex) {
            mutex->acquire();
        }
        fRelease = [](void* mutex) { ((T*)mutex)->release(); };
    }

    template <typename T>
    SkAutoMutexAcquire(T& mutex) : SkAutoMutexAcquire(&mutex) {}

    ~SkAutoMutexAcquire() { this->release(); }

    void release() {
        if (fMutex) {
            fRelease(fMutex);
        }
        fMutex = nullptr;
    }

private:
    void*  fMutex;
    void (*fRelease)(void*);
};
#define SkAutoMutexAcquire(...) SK_REQUIRE_LOCAL_VAR(SkAutoMutexAcquire)

// SkAutoExclusive is a lighter weight version of SkAutoMutexAcquire.
// It assumes that there is a valid mutex, obviating the null check.
class SkAutoExclusive {
public:
    template <typename T>
    SkAutoExclusive(T& mutex) : fMutex(&mutex) {
        mutex.acquire();

        fRelease = [](void* mutex) { ((T*)mutex)->release(); };
    }
    ~SkAutoExclusive() { fRelease(fMutex); }

private:
    void* fMutex;
    void (*fRelease)(void*);
};
#define SkAutoExclusive(...) SK_REQUIRE_LOCAL_VAR(SkAutoExclusive)

#endif//SkMutex_DEFINED
