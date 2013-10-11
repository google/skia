/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOnce_DEFINED
#define SkOnce_DEFINED

// SkOnce.h defines two macros, DEF_SK_ONCE and SK_ONCE.
// You can use these macros together to create a threadsafe block of code that
// runs at most once, no matter how many times you call it.  This is
// particularly useful for lazy singleton initialization.  E.g.
//
// DEF_SK_ONCE(set_up_my_singleton, SingletonType* singleton) {
//   // Code in this block will run at most once.
//   *singleton = new Singleton(...);
// }
// ...
// const Singleton& getSingleton() {
//   static Singleton* singleton = NULL;
//   // Always call SK_ONCE.  It's very cheap to call after the first time.
//   SK_ONCE(set_up_my_singleton, singleton);
//   SkASSERT(NULL != singleton);
//   return *singleton;
// }
//
// OnceTest.cpp also should serve as another simple example.

#include "SkThread.h"
#include "SkTypes.h"


// Pass a unique name (at least in this scope) for name, and a type and name
// for arg (as if writing a function declaration).
// E.g.
//   DEF_SK_ONCE(my_onetime_setup, int* foo) {
//     *foo += 5;
//   }
#define DEF_SK_ONCE(name, arg)                       \
    static bool sk_once_##name##_done = false;       \
    SK_DECLARE_STATIC_MUTEX(sk_once_##name##_mutex); \
    static void sk_once_##name##_function(arg)

// Call this anywhere you need to guarantee that the corresponding DEF_SK_ONCE
// block of code has run.  name should match the DEF_SK_ONCE, and here you pass
// the actual value of the argument.
// E.g
//   int foo = 0;
//   SK_ONCE(my_onetime_setup, &foo);
//   SkASSERT(5 == foo);
#define SK_ONCE(name, arg) \
    sk_once(&sk_once_##name##_done, &sk_once_##name##_mutex, sk_once_##name##_function, arg)


//  ----------------------  Implementation details below here. -----------------------------


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
    asm volatile("dmb" : : : "memory");
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
// This should be rarely called, so we separate it from sk_once and don't mark it as inline.
// (We don't mind if this is an actual function call, but odds are it'll be inlined anyway.)
template <typename Arg>
static void sk_once_slow(bool* done, SkBaseMutex* mutex, void (*once)(Arg), Arg arg) {
    const SkAutoMutexAcquire lock(*mutex);
    if (!*done) {
        once(arg);
        // Also known as a store-store/load-store barrier, this makes sure that the writes
        // done before here---in particular, those done by calling once(arg)---are observable
        // before the writes after the line, *done = true.
        //
        // In version control terms this is like saying, "check in the work up
        // to and including once(arg), then check in *done=true as a subsequent change".
        //
        // We'll use this in the fast path to make sure once(arg)'s effects are
        // observable whenever we observe *done == true.
        release_barrier();
        *done = true;
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
template <typename Arg>
inline static void sk_once(bool* done, SkBaseMutex* mutex, void (*once)(Arg), Arg arg) {
    ANNOTATE_BENIGN_RACE(done, "Don't worry TSAN, we're sure this is safe.");
    if (!*done) {
        sk_once_slow(done, mutex, once, arg);
    }
    // Also known as a load-load/load-store barrier, this acquire barrier makes
    // sure that anything we read from memory---in particular, memory written by
    // calling once(arg)---is at least as current as the value we read from done.
    //
    // In version control terms, this is a lot like saying "sync up to the
    // commit where we wrote *done = true".
    //
    // The release barrier in sk_once_slow guaranteed that *done = true
    // happens after once(arg), so by syncing to *done = true here we're
    // forcing ourselves to also wait until the effects of once(arg) are readble.
    acquire_barrier();
}

#undef ANNOTATE_BENIGN_RACE


#endif  // SkOnce_DEFINED
