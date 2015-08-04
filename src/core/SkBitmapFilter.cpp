/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapFilter.h"
#include "SkRTConf.h"
#include "SkTypes.h"

#include <string.h>

// These are the per-scanline callbacks that are used when we must resort to
// resampling an image as it is blitted.  Typically these are used only when
// the image is rotated or has some other complex transformation applied.
// Scaled images will usually be rescaled directly before rasterization.

SK_CONF_DECLARE(const char *, c_bitmapFilter, "bitmap.filter", "mitchell", "Which scanline bitmap filter to use [mitchell, lanczos, hamming, gaussian, triangle, box]");

SkBitmapFilter *SkBitmapFilter::Allocate() {
    if (!strcmp(c_bitmapFilter, "mitchell")) {
        return SkNEW_ARGS(SkMitchellFilter,(1.f/3.f,1.f/3.f));
    } else if (!strcmp(c_bitmapFilter, "lanczos")) {
        return SkNEW(SkLanczosFilter);
    } else if (!strcmp(c_bitmapFilter, "hamming")) {
        return SkNEW(SkHammingFilter);
    } else if (!strcmp(c_bitmapFilter, "gaussian")) {
        return SkNEW_ARGS(SkGaussianFilter,(2));
    } else if (!strcmp(c_bitmapFilter, "triangle")) {
        return SkNEW(SkTriangleFilter);
    } else if (!strcmp(c_bitmapFilter, "box")) {
        return SkNEW(SkBoxFilter);
    } else {
        SkDEBUGFAIL("Unknown filter type");
    }

    return NULL;
}

