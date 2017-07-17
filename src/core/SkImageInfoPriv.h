/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageInfoPriv_DEFINED
#define SkImageInfoPriv_DEFINED

#include "SkImageInfo.h"

/**
 *  This contains shared checks on SkImageInfo.  Depending on the desired color space behavior,
 *  the caller should choose one of the two versions below.
 */
static inline bool SkImageInfoIsValidCommon(const SkImageInfo& info) {
    if (info.width() <= 0 || info.height() <= 0) {
        return false;
    }

    const int kMaxDimension = SK_MaxS32 >> 2;
    if (info.width() > kMaxDimension || info.height() > kMaxDimension) {
        return false;
    }

    if (kUnknown_SkColorType == info.colorType() || kUnknown_SkAlphaType == info.alphaType()) {
        return false;
    }

    if (kOpaque_SkAlphaType != info.alphaType() &&
       (kRGB_565_SkColorType == info.colorType() || kGray_8_SkColorType == info.colorType())) {
        return false;
    }

    if (kRGBA_F16_SkColorType == info.colorType() &&
       (!info.colorSpace() || !info.colorSpace()->gammaIsLinear())) {
        return false;
    }

    return true;
}

/**
 *  Returns true if |info| contains a valid combination of width, height, colorType, alphaType,
 *  colorSpace.  Allows numerical color spaces.  Returns false otherwise.
 */
static inline bool SkImageInfoIsValidAllowNumericalCS(const SkImageInfo& info) {
    if (!SkImageInfoIsValidCommon(info)) {
        return false;
    }

    SkColorSpaceTransferFn fn;
    if (info.colorSpace() && !info.colorSpace()->isNumericalTransferFn(&fn)) {
        return false;
    }

    return true;
}

/**
 *  Returns true if |info| contains a valid combination of width, height, colorType, alphaType,
 *  colorSpace.  Only supports rendering color spaces.  Returns false otherwise.
 */
static inline bool SkImageInfoIsValidRenderingCS(const SkImageInfo& info) {
    if (!SkImageInfoIsValidCommon(info)) {
        return false;
    }

    if (info.colorSpace() &&
       (!info.colorSpace()->gammaCloseToSRGB() && !info.colorSpace()->gammaIsLinear())) {
        return false;
    }

    return true;
}

/**
 *  Returns true if |info| contains a valid combination of width, height, colorType, alphaType,
 *  colorSpace.  Uses |colorMode| to decide how to treat color spaces.
 */
static inline bool SkImageInfoIsValid(const SkImageInfo& info,
                                      SkDestinationSurfaceColorMode colorMode) {
    if (SkDestinationSurfaceColorMode::kGammaAndColorSpaceAware == colorMode) {
        return SkImageInfoIsValidRenderingCS(info);
    }

    return SkImageInfoIsValidAllowNumericalCS(info);
}

/**
 *  Returns true if Skia has defined a pixel conversion from the |src| to the |dst|.
 *  Returns false otherwise.  Some discussion of false cases:
 *      We will not convert to kIndex8 unless it exactly matches the src, since color tables
 *      are immutable.
 *      We do not convert to kGray8 when the |src| is not kGray8 in the same color space.
 *      We may add this feature - it just requires some work to convert to luminance while
 *      handling color spaces correctly.  Currently no one is asking for this.
 *      We will not convert from kAlpha8 when the |dst| is not kAlpha8.  This would require
 *      inventing color information.
 *      We will not convert to kOpaque when the |src| is not kOpaque.  This could be
 *      implemented to set all the alpha values to 1, but there is still some ambiguity -
 *      should we use kPremul or kUnpremul color values with the opaque alphas?  Or should
 *      we just use whatever the |src| alpha is?  In the future, we could choose to clearly
 *      define this, but currently no one is asking for this feature.
 *      We will not convert to a particular color space if |src| is nullptr.  The color space
 *      conversion is not well-defined.
 */
static inline bool SkImageInfoValidConversion(const SkImageInfo& dst, const SkImageInfo& src) {
    if (!SkImageInfoIsValidAllowNumericalCS(dst) || !SkImageInfoIsValidAllowNumericalCS(src)) {
        return false;
    }

    if (kGray_8_SkColorType == dst.colorType()) {
        if (kGray_8_SkColorType != src.colorType()) {
            return false;
        }

        if (dst.colorSpace() && !SkColorSpace::Equals(dst.colorSpace(), src.colorSpace())) {
            return false;
        }
    }

    if (kAlpha_8_SkColorType != dst.colorType() && kAlpha_8_SkColorType == src.colorType()) {
        return false;
    }

    if (kOpaque_SkAlphaType == dst.alphaType() && kOpaque_SkAlphaType != src.alphaType()) {
        return false;
    }

    if (dst.colorSpace() && !src.colorSpace()) {
        return false;
    }

    return true;
}
#endif  // SkImageInfoPriv_DEFINED
