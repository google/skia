/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnAsyncWait_DEFINED
#define skgpu_graphite_DawnAsyncWait_DEFINED

#include "webgpu/webgpu_cpp.h"

#include <atomic>
#include <functional>

namespace skgpu::graphite {

class DawnAsyncWait {
public:
    explicit DawnAsyncWait(const wgpu::Device& device);

    // Returns true if the wait has been signaled and false otherwise. This function yields
    // execution to the event loop where Dawn's asynchronous tasks get scheduled and returns
    // as soon as the loop yields the execution back to the caller.
    bool yieldAndCheck() const;

    // Busy-waits until this wait has been signaled.
    // TODO(armansito): This could benefit from a timeout in the case the wait never gets signaled.
    void busyWait() const;

    // Marks this wait as resolved. Once called, all calls to `yieldAndCheck` and `busyWait` will
    // return true immediately.
    void signal() { fSignaled.store(true, std::memory_order_release); }

    // Resets this object into its unsignaled state.
    void reset() { fSignaled.store(false, std::memory_order_release); }

private:
    wgpu::Device fDevice;
    std::atomic_bool fSignaled;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnAsyncWait_DEFINED
