/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXformPriv_DEFINED
#define SkColorSpaceXformPriv_DEFINED

#include "SkColorSpaceXform.h"

static inline float clamp_0_1(float v) {
    // The ordering of the logic is a little strange here in order
    // to make sure we convert NaNs to 0.
    if (v >= 1.0f) {
        return 1.0f;
    } else if (v >= 0.0f) {
        return v;
    } else {
        return 0.0f;
    }
}

static inline SkColorSpaceXform::ColorFormat select_xform_format(SkColorType colorType) {
    switch (colorType) {
        case kRGBA_8888_SkColorType:
            return SkColorSpaceXform::kRGBA_8888_ColorFormat;
        case kBGRA_8888_SkColorType:
            return SkColorSpaceXform::kBGRA_8888_ColorFormat;
        case kRGBA_F16_SkColorType:
            return SkColorSpaceXform::kRGBA_F16_ColorFormat;
        case kRGB_565_SkColorType:
            return SkColorSpaceXform::kBGR_565_ColorFormat;
        default:
            SkASSERT(false);
            return SkColorSpaceXform::kRGBA_8888_ColorFormat;
    }
}

#endif
