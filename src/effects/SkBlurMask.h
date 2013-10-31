
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBlurMask_DEFINED
#define SkBlurMask_DEFINED

#include "SkShader.h"
#include "SkMask.h"

class SkBlurMask {
public:
    enum Style {
        kNormal_Style,  //!< fuzzy inside and outside
        kSolid_Style,   //!< solid inside, fuzzy outside
        kOuter_Style,   //!< nothing inside, fuzzy outside
        kInner_Style,   //!< fuzzy inside, nothing outside

        kStyleCount
    };

    enum Quality {
        kLow_Quality,   //!< box blur
        kHigh_Quality   //!< three pass box blur (similar to gaussian)
    };

    static bool BlurRect(SkScalar sigma, SkMask *dst, const SkRect &src,
                         Style style,
                         SkIPoint *margin = NULL,
                         SkMask::CreateMode createMode =
                                                SkMask::kComputeBoundsAndRenderImage_CreateMode);
    static bool BoxBlur(SkMask* dst, const SkMask& src,
                        SkScalar sigma, Style style, Quality quality,
                        SkIPoint* margin = NULL);

    // the "ground truth" blur does a gaussian convolution; it's slow
    // but useful for comparison purposes.
    static bool BlurGroundTruth(SkScalar sigma, SkMask* dst, const SkMask& src,
                                Style style,
                                SkIPoint* margin = NULL);

    SK_ATTR_DEPRECATED("use sigma version")
    static bool BlurRect(SkMask *dst, const SkRect &src,
                         SkScalar radius, Style style,
                         SkIPoint *margin = NULL,
                         SkMask::CreateMode createMode =
                                                SkMask::kComputeBoundsAndRenderImage_CreateMode);

    SK_ATTR_DEPRECATED("use sigma version")
    static bool Blur(SkMask* dst, const SkMask& src,
                     SkScalar radius, Style style, Quality quality,
                     SkIPoint* margin = NULL);

    SK_ATTR_DEPRECATED("use sigma version")
    static bool BlurGroundTruth(SkMask* dst, const SkMask& src,
                                SkScalar radius, Style style,
                                SkIPoint* margin = NULL);

    static SkScalar ConvertRadiusToSigma(SkScalar radius);
};

#endif
