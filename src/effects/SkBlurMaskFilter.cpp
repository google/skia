/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurMaskFilter.h"

sk_sp<SkMaskFilter> SkBlurMaskFilter::Make(SkBlurStyle style, SkScalar sigma,
                                           const SkRect& occluder, uint32_t flags) {
    bool respectCTM = !(flags & kIgnoreTransform_BlurFlag);
    return SkMaskFilter::MakeBlur(style, sigma, occluder, respectCTM);
}
