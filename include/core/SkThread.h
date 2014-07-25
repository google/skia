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
static int64_t sk_atomic_inc(int64_t* addr);

/** Atomically adds inc to the int referenced by addr and returns the previous value.
 *  No additional memory barrier is required; this must act as a compiler barrier.
 */
static int32_t sk_atomic_add(int32_t* addr, int32_t inc);

/** Atomically subtracts one from the int referenced by addr and returns the previous value.
 *  This must act as a release (SL/S) memory barrier and as a compiler barrier.
 */
static int32_t sk_atomic_dec(int32_t* addr);

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

/** Atomically adds one to the int referenced by addr iff the referenced int was not 0
 *  and returns the previous value.
 *  No additional memory barrier is required; this must act as a compiler barrier.
 */
template<typename INT_TYPE> static inline INT_TYPE sk_atomic_conditional_inc(INT_TYPE* addr) {
    INT_TYPE prev;
    do {
        prev = *addr;
        if (0 == prev) {
            break;
        }
    } while (!sk_atomic_cas(addr, prev, prev+1));
    return prev;
}

// SK_BARRIERS_PLATFORM_H must provide implementations for the following declarations:

/** Prevent the compiler from reordering across this barrier. */
static void sk_compiler_barrier();

/** Read T*, with at least an acquire barrier.
 *
 *  Only needs to be implemented for T which can be atomically read.
 */
template <typename T> T sk_acquire_load(T*);

/** Write T*, with at least a release barrier.
 *
 *  Only needs to be implemented for T which can be atomically written.
 */
template <typename T> void sk_release_store(T*, T);

#include SK_BARRIERS_PLATFORM_H

/** SK_MUTEX_PLATFORM_H must provide the following (or equivalent) declarations.

class SkBaseMutex {
public:
    void acquire();     // Block until this thread owns the mutex.
    void release();     // Assuming this thread owns the mutex, release it.
    void assertHeld();  // If SK_DEBUG, assert this thread owns the mutex.
};

class SkMutex : SkBaseMutex {
public:
    SkMutex();
    ~SkMutex();
};

#define SK_DECLARE_STATIC_MUTEX(name) static SkBaseMutex name = ...
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

    /** Assert that we're holding the mutex. */
    void assertHeld() {
        SkASSERT(fMutex);
        fMutex->assertHeld();
    }

private:
    SkBaseMutex* fMutex;
};
#define SkAutoMutexAcquire(...) SK_REQUIRE_LOCAL_VAR(SkAutoMutexAcquire)

#endif
