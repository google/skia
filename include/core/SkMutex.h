/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMutex_DEFINED
#define SkMutex_DEFINED

// This file is not part of the public Skia API.
#include "SkTypes.h"

// IWYU pragma: begin_exports
#if defined(SK_BUILD_FOR_WIN)
    #include "../ports/SkMutex_win.h"
#else
    #include "../ports/SkMutex_pthread.h"
#endif
// IWYU pragma: end_exports

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
