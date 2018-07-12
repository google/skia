/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXformPriv_DEFINED
#define SkColorSpaceXformPriv_DEFINED

#include "SkColorSpaceXform.h"

std::unique_ptr<SkColorSpaceXform> SkMakeColorSpaceXform(SkColorSpace* src, SkColorSpace* dst);

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
