/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOnce_DEFINED
#define SkOnce_DEFINED

// SkOnce.h defines SK_DECLARE_STATIC_ONCE and SkOnce(), which you can use
// together to create a threadsafe way to call a function just once.  This
// is particularly useful for lazy singleton initialization. E.g.
//
// static void set_up_my_singleton(Singleton** singleton) {
//     *singleton = new Singleton(...);
// }
// ...
// const Singleton& GetSingleton() {
//     static Singleton* singleton = NULL;
//     SK_DECLARE_STATIC_ONCE(once);
//     SkOnce(&once, set_up_my_singleton, &singleton);
//     SkASSERT(NULL != singleton);
//     return *singleton;
// }
//
// OnceTest.cpp also should serve as a few other simple examples.

#include "SkThread.h"
#include "SkTypes.h"

#ifdef SK_USE_POSIX_THREADS
#  define SK_ONCE_INIT { false, { PTHREAD_MUTEX_INITIALIZER } }
#else
#  define SK_ONCE_INIT { false, SkBaseMutex() }
#endif

#define SK_DECLARE_STATIC_ONCE(name) static SkOnceFlag name = SK_ONCE_INIT

struct SkOnceFlag;  // If manually created, initialize with SkOnceFlag once = SK_ONCE_INIT

template <typename Func, typename Arg>
inline void SkOnce(SkOnceFlag* once, Func f, Arg arg);

//  ----------------------  Implementation details below here. -----------------------------

struct SkOnceFlag {
    bool done;
    SkBaseMutex mutex;
};

// TODO(bungeman, mtklein): move all these *barrier* functions to SkThread when refactoring lands.

#ifdef SK_BUILD_FOR_WIN
#include <intrin.h>
inline static void compiler_barrier() {
    _ReadWriteBarrier();
}
#else
inline static void compiler_barrier() {
    asm volatile("" : : : "memory");
}
#endif

inline static void full_barrier_on_arm() {
#ifdef SK_CPU_ARM
#if SK_ARM_ARCH >= 7
    asm volatile("dmb" : : : "memory");
#else
    asm volatile("mcr p15, 0, %0, c7, c10, 5" : : "r" (0) : "memory");
#endif
#endif
}

// On every platform, we issue a compiler barrier to prevent it from reordering
// code.  That's enough for platforms like x86 where release and acquire
// barriers are no-ops.  On other platforms we may need to be more careful;
// ARM, in particular, needs real code for both acquire and release.  We use a
// full barrier, which acts as both, because that the finest precision ARM
// provides.

inline static void release_barrier() {
    compiler_barrier();
    full_barrier_on_arm();
}

inline static void acquire_barrier() {
    compiler_barrier();
    full_barrier_on_arm();
}

// We've pulled a pretty standard double-checked locking implementation apart
// into its main fast path and a slow path that's called when we suspect the
// one-time code hasn't run yet.

// This is the guts of the code, called when we suspect the one-time code hasn't been run yet.
// This should be rarely called, so we separate it from SkOnce and don't mark it as inline.
// (We don't mind if this is an actual function call, but odds are it'll be inlined anyway.)
template <typename Func, typename Arg>
static void sk_once_slow(SkOnceFlag* once, Func f, Arg arg) {
    const SkAutoMutexAcquire lock(once->mutex);
    if (!once->done) {
        f(arg);
        // Also known as a store-store/load-store barrier, this makes sure that the writes
        // done before here---in particular, those done by calling f(arg)---are observable
        // before the writes after the line, *done = true.
        //
        // In version control terms this is like saying, "check in the work up
        // to and including f(arg), then check in *done=true as a subsequent change".
        //
        // We'll use this in the fast path to make sure f(arg)'s effects are
        // observable whenever we observe *done == true.
        release_barrier();
        once->done = true;
    }
}

// We nabbed this code from the dynamic_annotations library, and in their honor
// we check the same define.  If you find yourself wanting more than just
// ANNOTATE_BENIGN_RACE, it might make sense to pull that in as a dependency
// rather than continue to reproduce it here.

#if DYNAMIC_ANNOTATIONS_ENABLED
// TSAN provides this hook to supress a known-safe apparent race.
extern "C" {
void AnnotateBenignRace(const char* file, int line, const volatile void* mem, const char* desc);
}
#define ANNOTATE_BENIGN_RACE(mem, desc) AnnotateBenignRace(__FILE__, __LINE__, mem, desc)
#else
#define ANNOTATE_BENIGN_RACE(mem, desc)
#endif

// This is our fast path, called all the time.  We do really want it to be inlined.
template <typename Func, typename Arg>
inline void SkOnce(SkOnceFlag* once, Func f, Arg arg) {
    ANNOTATE_BENIGN_RACE(&(once->done), "Don't worry TSAN, we're sure this is safe.");
    if (!once->done) {
        sk_once_slow(once, f, arg);
    }
    // Also known as a load-load/load-store barrier, this acquire barrier makes
    // sure that anything we read from memory---in particular, memory written by
    // calling f(arg)---is at least as current as the value we read from once->done.
    //
    // In version control terms, this is a lot like saying "sync up to the
    // commit where we wrote once->done = true".
    //
    // The release barrier in sk_once_slow guaranteed that once->done = true
    // happens after f(arg), so by syncing to once->done = true here we're
    // forcing ourselves to also wait until the effects of f(arg) are readble.
    acquire_barrier();
}

#undef ANNOTATE_BENIGN_RACE

#endif  // SkOnce_DEFINED
