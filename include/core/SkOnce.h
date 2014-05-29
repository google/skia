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
//
// You may optionally pass SkOnce a second function to be called at exit for cleanup.

#include "SkDynamicAnnotations.h"
#include "SkThread.h"
#include "SkTypes.h"

#define SK_ONCE_INIT { false, { 0, SkDEBUGCODE(0) } }
#define SK_DECLARE_STATIC_ONCE(name) static SkOnceFlag name = SK_ONCE_INIT

struct SkOnceFlag;  // If manually created, initialize with SkOnceFlag once = SK_ONCE_INIT

template <typename Func, typename Arg>
inline void SkOnce(SkOnceFlag* once, Func f, Arg arg, void(*atExit)() = NULL);

// If you've already got a lock and a flag to use, this variant lets you avoid an extra SkOnceFlag.
template <typename Lock, typename Func, typename Arg>
inline void SkOnce(bool* done, Lock* lock, Func f, Arg arg, void(*atExit)() = NULL);

//  ----------------------  Implementation details below here. -----------------------------

// This is POD and must be zero-initialized.
struct SkSpinlock {
    void acquire() {
        SkASSERT(shouldBeZero == 0);
        // No memory barrier needed, but sk_atomic_cas gives us at least release anyway.
        while (!sk_atomic_cas(&thisIsPrivate, 0, 1)) {
            // spin
        }
    }

    void release() {
        SkASSERT(shouldBeZero == 0);
        // This requires a release memory barrier before storing, which sk_atomic_cas guarantees.
        SkAssertResult(sk_atomic_cas(&thisIsPrivate, 1, 0));
    }

    int32_t thisIsPrivate;
    SkDEBUGCODE(int32_t shouldBeZero;)
};

struct SkOnceFlag {
    bool done;
    SkSpinlock lock;
};

// Works with SkSpinlock or SkMutex.
template <typename Lock>
class SkAutoLockAcquire {
public:
    explicit SkAutoLockAcquire(Lock* lock) : fLock(lock) { fLock->acquire(); }
    ~SkAutoLockAcquire() { fLock->release(); }
private:
    Lock* fLock;
};

// We've pulled a pretty standard double-checked locking implementation apart
// into its main fast path and a slow path that's called when we suspect the
// one-time code hasn't run yet.

// This is the guts of the code, called when we suspect the one-time code hasn't been run yet.
// This should be rarely called, so we separate it from SkOnce and don't mark it as inline.
// (We don't mind if this is an actual function call, but odds are it'll be inlined anyway.)
template <typename Lock, typename Func, typename Arg>
static void sk_once_slow(bool* done, Lock* lock, Func f, Arg arg, void (*atExit)()) {
    const SkAutoLockAcquire<Lock> locked(lock);
    if (!*done) {
        f(arg);
        if (atExit != NULL) {
            atexit(atExit);
        }
        // Also known as a store-store/load-store barrier, this makes sure that the writes
        // done before here---in particular, those done by calling f(arg)---are observable
        // before the writes after the line, *done = true.
        //
        // In version control terms this is like saying, "check in the work up
        // to and including f(arg), then check in *done=true as a subsequent change".
        //
        // We'll use this in the fast path to make sure f(arg)'s effects are
        // observable whenever we observe *done == true.
        sk_release_store(done, true);
    }
}

// This is our fast path, called all the time.  We do really want it to be inlined.
template <typename Lock, typename Func, typename Arg>
inline void SkOnce(bool* done, Lock* lock, Func f, Arg arg, void(*atExit)()) {
    if (!SK_ANNOTATE_UNPROTECTED_READ(*done)) {
        sk_once_slow(done, lock, f, arg, atExit);
    }
    // Also known as a load-load/load-store barrier, this acquire barrier makes
    // sure that anything we read from memory---in particular, memory written by
    // calling f(arg)---is at least as current as the value we read from done.
    //
    // In version control terms, this is a lot like saying "sync up to the
    // commit where we wrote done = true".
    //
    // The release barrier in sk_once_slow guaranteed that done = true
    // happens after f(arg), so by syncing to done = true here we're
    // forcing ourselves to also wait until the effects of f(arg) are readble.
    SkAssertResult(sk_acquire_load(done));
}

template <typename Func, typename Arg>
inline void SkOnce(SkOnceFlag* once, Func f, Arg arg, void(*atExit)()) {
    return SkOnce(&once->done, &once->lock, f, arg, atExit);
}

#undef SK_ANNOTATE_BENIGN_RACE

#endif  // SkOnce_DEFINED
