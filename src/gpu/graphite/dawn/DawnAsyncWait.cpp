/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnAsyncWait.h"

#include <thread>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif  // __EMSCRIPTEN__

namespace skgpu::graphite {
namespace {

#ifdef __EMSCRIPTEN__

// When we use Dawn/WebGPU in a WebAssembly environment, we do not have access to
// `wgpu::Device::Tick()`, which is only available to dawn_native. Here we emulate the same
// behavior by scheduling and awaiting on a single async task, which will yield to the browser's
// underlying event loop.
//
// This requires that Emscripten is configured with `-s ASYNCIFY` to work as expected.
EM_ASYNC_JS(void, asyncSleep, (), {
    await new Promise((resolve, _) => {
        setTimeout(resolve, 0);
    })
});

#endif  // __EMSCRIPTEN__

}  // namespace

DawnAsyncWait::DawnAsyncWait(const wgpu::Device& device) : fDevice(device), fSignaled(false) {}

bool DawnAsyncWait::yieldAndCheck() const {
    if (fSignaled.load(std::memory_order_acquire)) {
        return true;
    }
#ifdef __EMSCRIPTEN__
    asyncSleep();
#else
    fDevice.Tick();
#endif  // __EMSCRIPTEN__
    return fSignaled.load(std::memory_order_acquire);
}

void DawnAsyncWait::busyWait() const {
    while (!this->yieldAndCheck()) {}
}

} // namespace skgpu::graphite
