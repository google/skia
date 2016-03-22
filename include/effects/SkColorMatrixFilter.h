/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorMatrixFilter_DEFINED
#define SkColorMatrixFilter_DEFINED

#include "SkColorFilter.h"
#include "SkColorMatrix.h"

class SK_API SkColorMatrixFilter : public SkColorFilter {
public:
    /**
     *  Create a colorfilter that multiplies the RGB channels by one color, and
     *  then adds a second color, pinning the result for each component to
     *  [0..255]. The alpha components of the mul and add arguments
     *  are ignored.
     */
    static sk_sp<SkColorFilter> MakeLightingFilter(SkColor mul, SkColor add);

#ifdef SK_SUPPORT_LEGACY_COLORFILTER_PTR
    static SkColorFilter* Create(const SkColorMatrix& cm) {
        return SkColorFilter::MakeMatrixFilterRowMajor255(cm.fMat).release();
    }
    static SkColorFilter* Create(const SkScalar array[20]) {
        return SkColorFilter::MakeMatrixFilterRowMajor255(array).release();
    }
    static SkColorFilter* CreateLightingFilter(SkColor mul, SkColor add) {
        return MakeLightingFilter(mul, add).release();
    }
#endif
};

#endif
