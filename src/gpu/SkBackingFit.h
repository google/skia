/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBackingFit_DEFINED
#define SkBackingFit_DEFINED

#include "include/core/SkSize.h"

/** Indicates whether a backing store needs to be an exact match or can be
    larger than is strictly necessary.
*/
enum class SkBackingFit { kApprox, kExact };

namespace skgpu {

/** Map dimensions to larger powers of 2. Above a certain tolerance,
    dimensions can also map to the midpoints between powers of 2.
 */
SkISize GetApproxSize(SkISize);

// Conceptually, when clamping a bilinearly filtered coordinate inside a valid domain (e.g. for
// an approx-fit reused scratch texture), it logically should be clamped to a 1/2 pixel inset from
// the texel subset bounds. However, when clamped to this inset texel center, it can be ambiguous
// whether or not linear filtering blends texel (i-1)@0 to i@1 or blends i@1 to (i+1)@0. For
// perfect precision and non-floating point formats, either of these should resolve to the value
// stored in texel i.
//
// For low-precision hardware and floating point formats, this is not necessarily the case.
// Sometimes the filter weight doesn't work out to exactly 0 for texel i+1 and it leaks, and if the
// value were not finite, it would also leak.
//
// Instead, we add an extra inset epsilon that ensures the clamped coordinate will always blend
// between an interior texel and the valid edge texel. The value is as small as possible without
// being lost in floating point precision for a max texture size of 16384. Alternatively, an
// epsilon could be calculated for each texture dimension but this allows it to remain a constant
// in shaders and keeps clamping behavior consistent across texture sizes.
//
// This is calculated as the smallest value `e` such that
//    (maxSize - 0.5 - e)/maxSize < (maxSize - 0.5)/maxSize -> e = ulp(maxSize-0.5)
//
// When maxSize = 16384, ulp(16383.5) ~ 1/512 for IEEE floating point numbers so we use that.
static constexpr float kInsetEpsilon = 1.f / 512.f;

// Value to use when insetting a approx-fit texture subset to clamp linear filtering to valid texels
static constexpr float kLinearInset = 0.5 + kInsetEpsilon;

}  // namespace skgpu

#endif
