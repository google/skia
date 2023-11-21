/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnAsyncWait_DEFINED
#define skgpu_graphite_DawnAsyncWait_DEFINED

#include "include/core/SkTypes.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

#include <atomic>
#include <functional>

namespace skgpu::graphite {

class DawnSharedContext;

class DawnAsyncWait {
public:
    DawnAsyncWait(const DawnSharedContext*);

    // Returns true if the wait has been signaled and false otherwise. If the Context allows
    // yielding, then this function yields execution to the event loop where Dawn's asynchronous
    // tasks get scheduled and returns as soon as the loop yields the execution back to the caller.
    // Otherwise, it just checks.
    bool yieldAndCheck() const;

    // Returns true if it is legal to call busyWait().
    bool mayBusyWait() const;

    // Busy-waits until this wait has been signaled. May only be called if the Context allows
    // yielding.
    // TODO(armansito): This could benefit from a timeout in the case the wait never gets signaled.
    void busyWait() const;

    // Marks this wait as resolved. Once called, all calls to `yieldAndCheck` and `busyWait` will
    // return true immediately.
    void signal() { fSignaled = true; }

    // Resets this object into its unsignaled state.
    void reset() { fSignaled = false; }

private:
    const DawnSharedContext* fSharedContext;
    std::atomic_bool fSignaled;
};

template <typename T> class DawnAsyncResult {
public:
    DawnAsyncResult(const DawnSharedContext* sharedContext) : fSync(sharedContext) {}

    ~DawnAsyncResult() {
        if (fSync.mayBusyWait()) {
            fSync.busyWait();
        }
        SkASSERT(fSync.yieldAndCheck());
    }

    void set(const T& result) {
        fResult = result;
        fSync.signal();
    }

    const T* getIfReady() const { return fSync.yieldAndCheck() ? &fResult : nullptr; }

    const T& waitAndGet() const {
        // If fSync is already signaled, the wait will return immediately.
        fSync.busyWait();
        return fResult;
    }

private:
    DawnAsyncWait fSync;
    T fResult;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnAsyncWait_DEFINED
