/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GpuTypesPriv_DEFINED
#define skgpu_GpuTypesPriv_DEFINED

#include "include/core/SkTypes.h"

#include <chrono>

namespace skgpu {

// The old libstdc++ uses the draft name "monotonic_clock" rather than "steady_clock". This might
// not actually be monotonic, depending on how libstdc++ was built. However, this is only currently
// used for idle resource purging so it shouldn't cause a correctness problem.
#if defined(__GLIBCXX__) && (__GLIBCXX__ < 20130000)
using StdSteadyClock = std::chrono::monotonic_clock;
#else
using StdSteadyClock = std::chrono::steady_clock;
#endif

} // namespace skgpu

#endif // skgpu_GpuTypesPriv_DEFINED
