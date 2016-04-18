/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOnce_DEFINED
#define SkOnce_DEFINED

#include "../private/SkSpinlock.h"
#include <atomic>
#include <utility>

// SkOnce provides call-once guarantees for Skia, much like std::once_flag/std::call_once().
//
// There should be no particularly error-prone gotcha use cases when using SkOnce.
// It works correctly as a class member, a local, a global, a function-scoped static, whatever.

class SkOnce {
public:
    template <typename Fn, typename... Args>
    void operator()(Fn&& fn, Args&&... args) {
        // Vanilla double-checked locking.
        if (!fDone.load(std::memory_order_acquire)) {
            fLock.acquire();
            if (!fDone.load(std::memory_order_relaxed)) {
                fn(std::forward<Args>(args)...);
                fDone.store(true, std::memory_order_release);
            }
            fLock.release();
        }
    }

private:
    std::atomic<bool> fDone{false};
    SkSpinlock        fLock;
};

#endif  // SkOnce_DEFINED
