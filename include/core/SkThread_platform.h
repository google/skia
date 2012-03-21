
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
static __attribute__((always_inline)) int32_t sk_atomic_inc(int32_t *addr) {
    return __sync_fetch_and_add(addr, 1);
}

static __attribute__((always_inline)) int32_t sk_atomic_dec(int32_t *addr) {
    return __sync_fetch_and_add(addr, -1);
}

#else // !SK_BUILD_FOR_ANDROID_NDK

/* The platform atomics operations are slightly more efficient than the
 * GCC built-ins, so use them.
 */
#include <utils/Atomic.h>

#define sk_atomic_inc(addr)     android_atomic_inc(addr)
#define sk_atomic_dec(addr)     android_atomic_dec(addr)

#endif // !SK_BUILD_FOR_ANDROID_NDK

#else  // !SK_BUILD_FOR_ANDROID

/** Implemented by the porting layer, this function adds 1 to the int specified
    by the address (in a thread-safe manner), and returns the previous value.
*/
SK_API int32_t sk_atomic_inc(int32_t* addr);
/** Implemented by the porting layer, this function subtracts 1 to the int
    specified by the address (in a thread-safe manner), and returns the previous
    value.
*/
SK_API int32_t sk_atomic_dec(int32_t* addr);

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

#define SK_DECLARE_STATIC_MUTEX(name)  static SkBaseMutex  name
#define SK_DECLARE_GLOBAL_MUTEX(name)  SkBaseMutex  name

#endif // !SK_USE_POSIX_THREADS


#endif
