/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/dawn/GrDawnAsyncWait.h"

#include "include/core/SkTypes.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif  // __EMSCRIPTEN__

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

GrDawnAsyncWait::GrDawnAsyncWait(const wgpu::Device& device) : fDevice(device), fSignaled(false) {}

bool GrDawnAsyncWait::yieldAndCheck() const {
    if (fSignaled.load()) {
        return true;
    }
#ifdef __EMSCRIPTEN__
    asyncSleep();
#else
    fDevice.Tick();
#endif  // __EMSCRIPTEN__
    return fSignaled.load();
}

void GrDawnAsyncWait::busyWait() const {
    while (!this->yieldAndCheck()) {}
}
