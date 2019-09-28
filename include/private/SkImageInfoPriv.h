/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageInfoPriv_DEFINED
#define SkImageInfoPriv_DEFINED

#include "include/core/SkImageInfo.h"

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
        case kUnknown_SkColorType:          return 0;
        case kAlpha_8_SkColorType:          return kAlpha_SkColorTypeComponentFlag;
        case kRGB_565_SkColorType:          return kRGB_SkColorTypeComponentFlags;
        case kARGB_4444_SkColorType:        return kRGBA_SkColorTypeComponentFlags;
        case kRGBA_8888_SkColorType:        return kRGBA_SkColorTypeComponentFlags;
        case kRGB_888x_SkColorType:         return kRGB_SkColorTypeComponentFlags;
        case kBGRA_8888_SkColorType:        return kRGBA_SkColorTypeComponentFlags;
        case kRGBA_1010102_SkColorType:     return kRGBA_SkColorTypeComponentFlags;
        case kRGB_101010x_SkColorType:      return kRGB_SkColorTypeComponentFlags;
        case kGray_8_SkColorType:           return kGray_SkColorTypeComponentFlag;
        case kRGBA_F16Norm_SkColorType:     return kRGBA_SkColorTypeComponentFlags;
        case kRGBA_F16_SkColorType:         return kRGBA_SkColorTypeComponentFlags;
        case kRGBA_F32_SkColorType:         return kRGBA_SkColorTypeComponentFlags;
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
        case kUnknown_SkColorType:          return 0;
        case kAlpha_8_SkColorType:          return 0;
        case kRGB_565_SkColorType:          return 1;
        case kARGB_4444_SkColorType:        return 1;
        case kRGBA_8888_SkColorType:        return 2;
        case kRGB_888x_SkColorType:         return 2;
        case kBGRA_8888_SkColorType:        return 2;
        case kRGBA_1010102_SkColorType:     return 2;
        case kRGB_101010x_SkColorType:      return 2;
        case kGray_8_SkColorType:           return 0;
        case kRGBA_F16Norm_SkColorType:     return 3;
        case kRGBA_F16_SkColorType:         return 3;
        case kRGBA_F32_SkColorType:         return 4;
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

    return true;
}

/**
 *  Returns true if Skia has defined a pixel conversion from the |src| to the |dst|.
 *  Returns false otherwise.
 */
static inline bool SkImageInfoValidConversion(const SkImageInfo& dst, const SkImageInfo& src) {
    return SkImageInfoIsValid(dst) && SkImageInfoIsValid(src);
}
#endif  // SkImageInfoPriv_DEFINED
