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
    static SkColorFilter* Create(const SkColorMatrix& cm) {
        return SkColorFilter::CreateMatrixFilterRowMajor255(cm.fMat);
    }
    static SkColorFilter* Create(const SkScalar array[20]) {
        return SkColorFilter::CreateMatrixFilterRowMajor255(array);
    }

    /**
     *  Create a colorfilter that multiplies the RGB channels by one color, and
     *  then adds a second color, pinning the result for each component to
     *  [0..255]. The alpha components of the mul and add arguments
     *  are ignored.
     */
    static SkColorFilter* CreateLightingFilter(SkColor mul, SkColor add);
};

#endif
