/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GradientBitmap_DEFINED
#define skgpu_GradientBitmap_DEFINED

#include "include/core/SkBitmap.h"
#include "src/core/SkColorData.h"

namespace skgpu {

// Gradient stop offsets / interval thresholds live in [0, 1] but lose too much
// precision when rounded to half floats directly. Encoding them as the
// (mantissa, exponent) pair returned by frexp preserves the value across an F16
// texture; the shader recovers it with ldexp(mantissa, exponent).
//
// On success, writes the mantissa and exponent and returns true. Returns false
// if the exponent does not round-trip through F16, in which case the caller must
// not use the texture encoding.
bool EncodeGradientStopToHalf(float offset, float* mantissa, float* exponent);

// Builds the F16 texture both Ganesh and Graphite upload for many-stop
// gradients. The bitmap is `numStops` wide and 2 tall: row 0 holds the premul
// colors and row 1 holds each stop offset encoded as (mantissa, exponent, 0, 1)
// via EncodeGradientStopToHalf. The shader binary-searches the offset row and
// mixes the two adjacent colors. If `offsets` is null the stops are assumed to
// be evenly spaced. Returns an empty bitmap if allocation or offset encoding
// fails.
SkBitmap CreateGradientColorAndOffsetBitmap(int numStops,
                                            const SkPMColor4f* colors,
                                            const float* offsets);

}  // namespace skgpu

#endif  // skgpu_GradientBitmap_DEFINED
