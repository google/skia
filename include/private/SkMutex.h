/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMutex_DEFINED
#define SkMutex_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkMacros.h"
#include "include/private/SkSemaphore.h"
#include "include/private/SkThreadAnnotations.h"
#include "include/private/SkThreadID.h"

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

class SK_CAPABILITY("mutex") SkMutex {
public:
    constexpr SkMutex() = default;

    void acquire() SK_ACQUIRE() {
        fSemaphore.wait();
        SkDEBUGCODE(fOwner = SkGetThreadID();)
    }

    void release() SK_RELEASE_CAPABILITY() {
        this->assertHeld();
        SkDEBUGCODE(fOwner = kIllegalThreadID;)
        fSemaphore.signal();
    }

    void assertHeld() SK_ASSERT_CAPABILITY(this) {
        SkASSERT(fOwner == SkGetThreadID());
    }

private:
    SkSemaphore fSemaphore{1};
    SkDEBUGCODE(SkThreadID fOwner{kIllegalThreadID};)
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

class SK_SCOPED_CAPABILITY SkAutoMutexExclusive {
public:
    SkAutoMutexExclusive(SkMutex& mutex) SK_ACQUIRE(mutex) : fMutex(mutex) { fMutex.acquire(); }
    ~SkAutoMutexExclusive() SK_RELEASE_CAPABILITY() { fMutex.release(); }

private:
    SkMutex& fMutex;
};

#define SkAutoMutexExclusive(...) SK_REQUIRE_LOCAL_VAR(SkAutoMutexExclusive)

#endif//SkMutex_DEFINED
