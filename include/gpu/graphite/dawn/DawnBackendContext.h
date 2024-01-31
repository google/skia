/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnBackendContext_DEFINED
#define skgpu_graphite_DawnBackendContext_DEFINED

#include "include/core/SkTypes.h"
#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

namespace skgpu::graphite {

/**
 * WebGPU needs to allow the main thread loop to run to detect GPU progress. Dawn native has a
 * function wgpu::Dawn::Tick(), not present in WebGPU, that can be used to detect GPU progress.
 *
 * When compiling using Emscripten/WASM the -s ASYNCIFY option can be used to yield. E.g.:
 *
 * EM_ASYNC_JS(void, asyncSleep, (), {
 *                 await new Promise((resolve, _) = > {
 *                     setTimeout(resolve, 0);
 *                 })
 *             });
 *
 * WebGPUTickFunction(wgpu::Device&) { asyncSleep(); }
 *
 * If no DawnTickFunction is provided then the graphite::Context will be "non-yielding". This
 * implies the following restrictions on the Context:
 *
 * 1) SyncToCpu::kYes is disallowed as a parameter to Context::submit.
 * 2) The client must guarantee that GPU work has completed before destroying Context as Context
 *    cannot await the work completion in its destructor. Context reports whether it is awaiting
 *    GPU work completion via Context::hasUnfinishedGpuWork().
 *
 * Using a non-yielding Context makes it possible to build and run Graphite/Dawn on WebGPU without
 * -s ASYNCIFY.
 */
using DawnTickFunction = void(const wgpu::Device& device);

#if !defined(__EMSCRIPTEN__)
SK_API inline void DawnNativeTickFunction(const wgpu::Device& device) { device.Tick(); }
#endif

// The DawnBackendContext contains all of the base Dawn objects needed by the graphite Dawn
// backend. The client will create this object and pass it into the Context::MakeDawn factory call
// when setting up Skia.
struct SK_API DawnBackendContext {
    wgpu::Instance fInstance;
    wgpu::Device fDevice;
    wgpu::Queue fQueue;
    // See comment on DawnTickFunction.
    DawnTickFunction* fTick =
#if defined(__EMSCRIPTEN__)
            nullptr;
#else
            DawnNativeTickFunction;
#endif
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnBackendContext_DEFINED
