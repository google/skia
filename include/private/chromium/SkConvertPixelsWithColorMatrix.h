/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkConvertPixelsWithColorMatrix_DEFINED
#define SkConvertPixelsWithColorMatrix_DEFINED

#include "include/private/SkAPI.h"

class SkColorMatrix;
class SkPixmap;

/**
 * Convert pixels from `src` to `dst`, optionally applying `srcMatrix` to unpremultiplied `src`
 * pixels before color space conversion, and `dstMatrix` to unpremultiplied `dst` pixels after
 * color space conversion. After each matrix application, the alpha channel is clamped to [0, 1],
 * while the RGB channels are not clamped (beyond any clamping performed when storing to `dst`).
 */
SK_API bool SkConvertPixelsWithColorMatrix(const SkPixmap& src,
                                           const SkColorMatrix* srcMatrix,
                                           const SkPixmap& dst,
                                           const SkColorMatrix* dstMatrix);

#endif  // SkConvertPixelsWithColorMatrix_DEFINED
