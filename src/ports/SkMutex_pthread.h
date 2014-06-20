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

// A SkBaseMutex is a POD structure that can be directly initialized
// at declaration time with SK_DECLARE_STATIC/GLOBAL_MUTEX. This avoids the
// generation of a static initializer in the final machine code (and
// a corresponding static finalizer).
struct SkBaseMutex {
    void acquire() {
        SkASSERT(fOwner != pthread_self());  // SkMutex is not re-entrant
        pthread_mutex_lock(&fMutex);
        SkDEBUGCODE(fOwner = pthread_self();)
    }
    void release() {
        this->assertHeld();
        SkDEBUGCODE(fOwner = 0;)
        pthread_mutex_unlock(&fMutex);
    }
    void assertHeld() {
        SkASSERT(pthread_self() == fOwner);
    }

    pthread_mutex_t fMutex;
    SkDEBUGCODE(pthread_t fOwner;)
};

// A normal mutex that requires to be initialized through normal C++ construction,
// i.e. when it's a member of another class, or allocated on the heap.
class SkMutex : public SkBaseMutex {
public:
    SkMutex() {
        SkDEBUGCODE(int status = )pthread_mutex_init(&fMutex, NULL);
        SkDEBUGCODE(
            if (status != 0) {
                print_pthread_error(status);
                SkASSERT(0 == status);
            }
        )
    }

    ~SkMutex() {
        SkDEBUGCODE(int status = )pthread_mutex_destroy(&fMutex);
        SkDEBUGCODE(
            if (status != 0) {
                print_pthread_error(status);
                SkASSERT(0 == status);
            }
        )
    }

private:
    SkMutex(const SkMutex&);
    SkMutex& operator=(const SkMutex&);

    static void print_pthread_error(int status) {
        switch (status) {
        case 0: // success
            break;
        case EINVAL:
            SkDebugf("pthread error [%d] EINVAL\n", status);
            break;
        case EBUSY:
            SkDebugf("pthread error [%d] EBUSY\n", status);
            break;
        default:
            SkDebugf("pthread error [%d] unknown\n", status);
            break;
        }
    }
};

#define SK_BASE_MUTEX_INIT { PTHREAD_MUTEX_INITIALIZER, SkDEBUGCODE(0) }

// Using POD-style initialization prevents the generation of a static initializer.
#define SK_DECLARE_STATIC_MUTEX(name) static SkBaseMutex name = SK_BASE_MUTEX_INIT

// Special case used when the static mutex must be available globally.
#define SK_DECLARE_GLOBAL_MUTEX(name) SkBaseMutex name = SK_BASE_MUTEX_INIT

#endif
