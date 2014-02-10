/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkThread_DEFINED
#define SkThread_DEFINED

#include "SkTypes.h"

// SK_ATOMICS_PLATFORM_H must provide inline implementations for the following declarations.

/** Atomically adds one to the int referenced by addr and returns the previous value.
 *  No additional memory barrier is required; this must act as a compiler barrier.
 */
static int32_t sk_atomic_inc(int32_t* addr);

/** Atomically adds inc to the int referenced by addr and returns the previous value.
 *  No additional memory barrier is required; this must act as a compiler barrier.
 */
static int32_t sk_atomic_add(int32_t* addr, int32_t inc);

/** Atomically subtracts one from the int referenced by addr and returns the previous value.
 *  This must act as a release (SL/S) memory barrier and as a compiler barrier.
 */
static int32_t sk_atomic_dec(int32_t* addr);

/** Atomically adds one to the int referenced by addr iff the referenced int was not 0
 *  and returns the previous value.
 *  No additional memory barrier is required; this must act as a compiler barrier.
 */
static int32_t sk_atomic_conditional_inc(int32_t* addr);

/** Atomic compare and set.
 *  If *addr == before, set *addr to after and return true, otherwise return false.
 *  This must act as a release (SL/S) memory barrier and as a compiler barrier.
 */
static bool sk_atomic_cas(int32_t* addr, int32_t before, int32_t after);

/** If sk_atomic_dec does not act as an acquire (L/SL) barrier,
 *  this must act as an acquire (L/SL) memory barrier and as a compiler barrier.
 */
static void sk_membar_acquire__after_atomic_dec();

/** If sk_atomic_conditional_inc does not act as an acquire (L/SL) barrier,
 *  this must act as an acquire (L/SL) memory barrier and as a compiler barrier.
 */
static void sk_membar_acquire__after_atomic_conditional_inc();

#include SK_ATOMICS_PLATFORM_H

/** SK_MUTEX_PLATFORM_H must provide the following (or equivalent) declarations.

class SkBaseMutex {
public:
    void acquire();
    void release();
};

class SkMutex : SkBaseMutex {
public:
    SkMutex();
    ~SkMutex();
};

#define SK_DECLARE_STATIC_MUTEX(name) static SkBaseMutex name = ...
#define SK_DECLARE_GLOBAL_MUTEX(name) SkBaseMutex name = ...
*/

#include SK_MUTEX_PLATFORM_H


class SkAutoMutexAcquire : SkNoncopyable {
public:
    explicit SkAutoMutexAcquire(SkBaseMutex& mutex) : fMutex(&mutex) {
        SkASSERT(fMutex != NULL);
        mutex.acquire();
    }

    explicit SkAutoMutexAcquire(SkBaseMutex* mutex) : fMutex(mutex) {
        if (mutex) {
            mutex->acquire();
        }
    }

    /** If the mutex has not been released, release it now. */
    ~SkAutoMutexAcquire() {
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

private:
    SkBaseMutex* fMutex;
};
#define SkAutoMutexAcquire(...) SK_REQUIRE_LOCAL_VAR(SkAutoMutexAcquire)

#endif
