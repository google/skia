/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOncePtr_DEFINED
#define SkOncePtr_DEFINED

#include "SkAtomics.h"

template <typename T> class SkStaticOnce;

// Use this to create a global static pointer that's intialized exactly once when you call get().
#define SK_DECLARE_STATIC_ONCE_PTR(type, name) namespace {} static SkStaticOnce<type> name

// Use this for a local or member pointer that's initialized exactly once when you call get().
template <typename T>
class SkOncePtr : SkNoncopyable {
public:
    SkOncePtr() { sk_bzero(this, sizeof(*this)); }

    // SkOncePtr does not have a destructor and does not clean up the pointer.  But you may, e.g.
    //   delete (T*)fOncePtr;
    //   SkSafeUnref((T*)fOncePtr);
    // etc.

    template <typename F>
    T* get(const F& f) const {
        return fOnce.get(f);
    }

    operator T*() const {
        return (T*)fOnce;
    }

private:
    SkStaticOnce<T> fOnce;
};

/* TODO(mtklein): in next CL
typedef SkStaticOnce<void> SkOnceFlag;
#define SK_DECLARE_STATIC_ONCE(name) namespace {} static SkOnceFlag name

template <typename F>
inline void SkOnce(SkOnceFlag* once, const F& f) {
    once->get([&]{ f(); return (void*)2; });
}
*/

// Implementation details below here!  No peeking!

template <typename T>
class SkStaticOnce {
public:
    template <typename F>
    T* get(const F& f) const {
        uintptr_t state = fState.load(sk_memory_order_acquire);
        if (state < 2) {
            if (state == 0) {
                // It looks like no one has tried to create our pointer yet.
                // We try to claim that task by atomically swapping our state from '0' to '1'.
                if (fState.compare_exchange(&state, 1, sk_memory_order_relaxed,
                                                       sk_memory_order_relaxed)) {
                    // We've claimed it.  Create our pointer and store it into fState.
                    state = (uintptr_t)f();
                    SkASSERT(state > 1);
                    fState.store(state, sk_memory_order_release);
                } else {
                    // Someone else claimed it.
                    // We fall through to the spin loop just below to wait for them to finish.
                }
            }

            while (state == 1) {
                // State '1' is our busy-but-not-done state.
                // Some other thread has claimed the job of creating our pointer.
                // We just need to wait for it to finish.
                state = fState.load(sk_memory_order_acquire);
            }

            // We shouldn't be able to get here without having created our pointer.
            SkASSERT(state > 1);
        }
        return (T*)state;
    }

    operator T*() const {
        auto state = fState.load(sk_memory_order_acquire);
        return state < 2 ? nullptr : (T*)state;
        // TODO: If state == 1 spin until it's not?
    }

private:
    // fState == 0 --> we have not created our ptr yet
    // fState == 1 --> someone is in the middle of creating our ptr
    // else        --> (T*)fState is our ptr
    mutable SkAtomic<uintptr_t> fState;
};

#endif//SkOncePtr_DEFINED
