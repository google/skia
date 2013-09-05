/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShader.h"
#include "SkBitmap.h"
#include "SkRegion.h"
#include "SkString.h"

class SK_API SkBitmapAlphaThresholdShader : public SkShader {
public:
    /**
     * Creates a shader that samples a bitmap and a region. If the sample is inside the region
     * the alpha of the bitmap color is boosted up to a threshold value. If it is
     * outside the region then the bitmap alpha is decreased to the threshold value.
     * The 0,0 point of the region corresponds to the upper left corner of the bitmap
     * Currently, this only has a GPU implementation, doesn't respect the paint's bitmap
     * filter setting, and always uses clamp mode.
     */
    static SkShader* Create(const SkBitmap& bitmap, const SkRegion& region, U8CPU threshold);
};
