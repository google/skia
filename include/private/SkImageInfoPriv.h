/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageInfoPriv_DEFINED
#define SkImageInfoPriv_DEFINED

#include "SkImageInfo.h"

enum SkColorTypeComponentFlag {
    kRed_SkColorTypeComponentFlag    = 0x1,
    kGreen_SkColorTypeComponentFlag  = 0x2,
    kBlue_SkColorTypeComponentFlag   = 0x4,
    kAlpha_SkColorTypeComponentFlag  = 0x8,
    kGray_SkColorTypeComponentFlag   = 0x10,
    kRGB_SkColorTypeComponentFlags   = kRed_SkColorTypeComponentFlag |
                                       kGreen_SkColorTypeComponentFlag |
                                       kBlue_SkColorTypeComponentFlag,
    kRGBA_SkColorTypeComponentFlags  = kRGB_SkColorTypeComponentFlags |
                                       kAlpha_SkColorTypeComponentFlag,
};

static inline uint32_t SkColorTypeComponentFlags(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:      return 0;
        case kAlpha_8_SkColorType:      return kAlpha_SkColorTypeComponentFlag;
        case kRGB_565_SkColorType:      return kRGB_SkColorTypeComponentFlags;
        case kARGB_4444_SkColorType:    return kRGBA_SkColorTypeComponentFlags;
        case kRGBA_8888_SkColorType:    return kRGBA_SkColorTypeComponentFlags;
        case kRGB_888x_SkColorType:     return kRGB_SkColorTypeComponentFlags;
        case kBGRA_8888_SkColorType:    return kRGBA_SkColorTypeComponentFlags;
        case kRGBA_1010102_SkColorType: return kRGBA_SkColorTypeComponentFlags;
        case kRGB_101010x_SkColorType:  return kRGB_SkColorTypeComponentFlags;
        case kGray_8_SkColorType:       return kGray_SkColorTypeComponentFlag;
        case kRGBA_F16_SkColorType:     return kRGBA_SkColorTypeComponentFlags;
        case kRGBA_F32_SkColorType:     return kRGBA_SkColorTypeComponentFlags;
    }
    return 0;
}

static inline bool SkColorTypeIsAlphaOnly(SkColorType ct) {
    return kAlpha_SkColorTypeComponentFlag == SkColorTypeComponentFlags(ct);
}

static inline bool SkAlphaTypeIsValid(unsigned value) {
    return value <= kLastEnum_SkAlphaType;
}

static inline bool SkColorTypeIsGray(SkColorType ct) {
    auto flags = SkColorTypeComponentFlags(ct);
    // Currently assuming that a color type has only gray or does not have gray.
    SkASSERT(!(kGray_SkColorTypeComponentFlag & flags) || kGray_SkColorTypeComponentFlag == flags);
    return kGray_SkColorTypeComponentFlag == flags;
}

static int SkColorTypeShiftPerPixel(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:      return 0;
        case kAlpha_8_SkColorType:      return 0;
        case kRGB_565_SkColorType:      return 1;
        case kARGB_4444_SkColorType:    return 1;
        case kRGBA_8888_SkColorType:    return 2;
        case kRGB_888x_SkColorType:     return 2;
        case kBGRA_8888_SkColorType:    return 2;
        case kRGBA_1010102_SkColorType: return 2;
        case kRGB_101010x_SkColorType:  return 2;
        case kGray_8_SkColorType:       return 0;
        case kRGBA_F16_SkColorType:     return 3;
        case kRGBA_F32_SkColorType:     return 4;
    }
    return 0;
}

static inline size_t SkColorTypeMinRowBytes(SkColorType ct, int width) {
    return width * SkColorTypeBytesPerPixel(ct);
}

static inline bool SkColorTypeIsValid(unsigned value) {
    return value <= kLastEnum_SkColorType;
}

static inline size_t SkColorTypeComputeOffset(SkColorType ct, int x, int y, size_t rowBytes) {
    if (kUnknown_SkColorType == ct) {
        return 0;
    }
    return y * rowBytes + (x << SkColorTypeShiftPerPixel(ct));
}

/**
 *  Returns true if |info| contains a valid combination of width, height, colorType, and alphaType.
 */
static inline bool SkImageInfoIsValid(const SkImageInfo& info) {
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

    return true;
}

/**
 *  Returns true if Skia has defined a pixel conversion from the |src| to the |dst|.
 *  Returns false otherwise.  Some discussion of false cases:
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
    if (!SkImageInfoIsValid(dst) || !SkImageInfoIsValid(src)) {
        return false;
    }

    if (SkColorTypeIsGray(dst.colorType())) {
        if (!SkColorTypeIsGray(src.colorType())) {
            return false;
        }

        if (dst.colorSpace() && !SkColorSpace::Equals(dst.colorSpace(), src.colorSpace())) {
            return false;
        }
    }

    if (!SkColorTypeIsAlphaOnly(dst.colorType()) && SkColorTypeIsAlphaOnly(src.colorType())) {
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
