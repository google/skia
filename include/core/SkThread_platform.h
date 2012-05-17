
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkThread_platform_DEFINED
#define SkThread_platform_DEFINED

#if defined(SK_BUILD_FOR_ANDROID)

#if defined(SK_BUILD_FOR_ANDROID_NDK)

#include <stdint.h>

/* Just use the GCC atomic intrinsics. They're supported by the NDK toolchain,
 * have reasonable performance, and provide full memory barriers
 */
static inline __attribute__((always_inline)) int32_t sk_atomic_inc(int32_t *addr) {
    return __sync_fetch_and_add(addr, 1);
}

static inline __attribute__((always_inline)) int32_t sk_atomic_dec(int32_t *addr) {
    return __sync_fetch_and_add(addr, -1);
}
static inline __attribute__((always_inline)) void sk_membar_aquire__after_atomic_dec() { }

static inline __attribute__((always_inline)) int32_t sk_atomic_conditional_inc(int32_t* addr) {
    int32_t value = *addr;

    while (true) {
        if (value == 0) {
            return 0;
        }

        int32_t before = __sync_val_compare_and_swap(addr, value, value + 1);

        if (before == value) {
            return value;
        } else {
            value = before;
        }
    }
}
static inline __attribute__((always_inline)) void sk_membar_aquire__after_atomic_conditional_inc() { }

#else // !SK_BUILD_FOR_ANDROID_NDK

/* The platform atomics operations are slightly more efficient than the
 * GCC built-ins, so use them.
 */
#include <utils/Atomic.h>

#define sk_atomic_inc(addr)     android_atomic_inc(addr)
#define sk_atomic_dec(addr)     android_atomic_dec(addr)
void sk_membar_aquire__after_atomic_dec() {
    //HACK: Android is actually using full memory barriers.
    //      Should this change, uncomment below.
    //int dummy;
    //android_atomic_aquire_store(0, &dummy);
}
int32_t sk_atomic_conditional_inc(int32_t* addr) {
    while (true) {
        int32_t value = *addr;
        if (value == 0) {
            return 0;
        }
        if (0 == android_atomic_release_cas(value, value + 1, addr)) {
            return value;
        }
    }
}
void sk_membar_aquire__after_atomic_conditional_inc() {
    //HACK: Android is actually using full memory barriers.
    //      Should this change, uncomment below.
    //int dummy;
    //android_atomic_aquire_store(0, &dummy);
}

#endif // !SK_BUILD_FOR_ANDROID_NDK

#else  // !SK_BUILD_FOR_ANDROID

/** Implemented by the porting layer, this function adds one to the int
    specified by the address (in a thread-safe manner), and returns the
    previous value.
    No additional memory barrier is required.
    This must act as a compiler barrier.
*/
SK_API int32_t sk_atomic_inc(int32_t* addr);

/** Implemented by the porting layer, this function subtracts one from the int
    specified by the address (in a thread-safe manner), and returns the
    previous value.
    Expected to act as a release (SL/S) memory barrier and a compiler barrier.
*/
SK_API int32_t sk_atomic_dec(int32_t* addr);
/** If sk_atomic_dec does not act as an aquire (L/SL) barrier, this is expected
    to act as an aquire (L/SL) memory barrier and as a compiler barrier.
*/
SK_API void sk_membar_aquire__after_atomic_dec();

/** Implemented by the porting layer, this function adds one to the int
    specified by the address iff the int specified by the address is not zero
    (in a thread-safe manner), and returns the previous value.
    No additional memory barrier is required.
    This must act as a compiler barrier.
*/
SK_API int32_t sk_atomic_conditional_inc(int32_t*);
/** If sk_atomic_conditional_inc does not act as an aquire (L/SL) barrier, this
    is expected to act as an aquire (L/SL) memory barrier and as a compiler
    barrier.
*/
SK_API void sk_membar_aquire__after_atomic_conditional_inc();

#endif // !SK_BUILD_FOR_ANDROID

#ifdef SK_USE_POSIX_THREADS

#include <pthread.h>

// A SkBaseMutex is a POD structure that can be directly initialized
// at declaration time with SK_DECLARE_STATIC/GLOBAL_MUTEX. This avoids the
// generation of a static initializer in the final machine code (and
// a corresponding static finalizer).
//
struct SkBaseMutex {
    void    acquire() { pthread_mutex_lock(&fMutex); }
    void    release() { pthread_mutex_unlock(&fMutex); }
    pthread_mutex_t  fMutex;
};

// Using POD-style initialization prevents the generation of a static initializer
// and keeps the acquire() implementation small and fast.
#define SK_DECLARE_STATIC_MUTEX(name)   static SkBaseMutex  name = { PTHREAD_MUTEX_INITIALIZER }

// Special case used when the static mutex must be available globally.
#define SK_DECLARE_GLOBAL_MUTEX(name)   SkBaseMutex  name = { PTHREAD_MUTEX_INITIALIZER }

#define SK_DECLARE_MUTEX_ARRAY(name, count)    SkBaseMutex name[count] = { PTHREAD_MUTEX_INITIALIZER }

// A normal mutex that requires to be initialized through normal C++ construction,
// i.e. when it's a member of another class, or allocated on the heap.
class SkMutex : public SkBaseMutex, SkNoncopyable {
public:
    SkMutex();
    ~SkMutex();
};

#else // !SK_USE_POSIX_THREADS

// In the generic case, SkBaseMutex and SkMutex are the same thing, and we
// can't easily get rid of static initializers.
//
class SkMutex : SkNoncopyable {
public:
    SkMutex();
    ~SkMutex();

    void    acquire();
    void    release();

private:
    bool fIsGlobal;
    enum {
        kStorageIntCount = 64
    };
    uint32_t    fStorage[kStorageIntCount];
};

typedef SkMutex SkBaseMutex;

#define SK_DECLARE_STATIC_MUTEX(name)           static SkBaseMutex  name
#define SK_DECLARE_GLOBAL_MUTEX(name)           SkBaseMutex  name
#define SK_DECLARE_MUTEX_ARRAY(name, count)     SkBaseMutex name[count]

#endif // !SK_USE_POSIX_THREADS


#endif
