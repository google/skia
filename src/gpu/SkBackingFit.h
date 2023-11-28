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

}  // namespace skgpu

#endif
