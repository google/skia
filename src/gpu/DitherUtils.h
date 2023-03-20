/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_DitherUtils_DEFINED
#define skgpu_DitherUtils_DEFINED

#include "include/core/SkTypes.h"

#ifndef SK_IGNORE_GPU_DITHER

enum SkColorType : int;
class SkBitmap;

namespace skgpu {

float DitherRangeForConfig(SkColorType dstColorType);

SkBitmap MakeDitherLUT();

} // namespace skgpu

#endif // SK_IGNORE_GPU_DITHER

#endif // skgpu_DitherUtils_DEFINED
