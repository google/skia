/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOnce_DEFINED
#define SkOnce_DEFINED

#include <atomic>
#include <utility>
#include "SkTypes.h"

// SkOnce provides call-once guarantees for Skia, much like std::once_flag/std::call_once().
//
// There should be no particularly error-prone gotcha use cases when using SkOnce.
// It works correctly as a class member, a local, a global, a function-scoped static, whatever.

class SkOnce {
public:
    constexpr SkOnce() = default;

    template <typename Fn, typename... Args>
    void operator()(Fn&& fn, Args&&... args) {
        auto state = fState.load(std::memory_order_acquire);

        if (state == Done) {
            return;
        }

        if (state == NotStarted) {
            // Try to claim the job of calling fn() by swapping from NotStarted to Calling.
            // See [1] below for why we use std::memory_order_acquire instead of relaxed.
            if (fState.compare_exchange_strong(state, Calling, std::memory_order_acquire)) {
                // Claimed!  Call fn(), then mark this SkOnce as Done.
                fn(std::forward<Args>(args)...);
                return fState.store(Done, std::memory_order_release);
            }
        }

        while (state == Calling) {
            // Some other thread is calling fn().  Wait for them to finish.
            state = fState.load(std::memory_order_acquire);
        }
        SkASSERT(state == Done);
    }

private:
    enum State : uint8_t { NotStarted, Calling, Done};
    std::atomic<uint8_t> fState{NotStarted};
};

/* [1]  Why do we compare_exchange_strong() with std::memory_order_acquire instead of relaxed?
 *
 * If we succeed, we really only need a relaxed compare_exchange_strong()... we're the ones
 * who are about to do a release store, so there's certainly nothing yet for an acquire to
 * synchronize with.
 *
 * If that compare_exchange_strong() fails, we're either in Calling or Done state.
 * Again, if we're in Calling state, relaxed would have been fine: the spin loop will
 * acquire up to the Calling thread's release store.
 *
 * But if that compare_exchange_strong() fails and we find ourselves in the Done state,
 * we've never done an acquire load to sync up to the store of that Done state.
 *
 * So on failure we need an acquire load.  Generally the failure memory order cannot be
 * stronger than the success memory order, so we need acquire on success too.  The single
 * memory order version of compare_exchange_strong() uses the same acquire order for both.
 */

#endif  // SkOnce_DEFINED
