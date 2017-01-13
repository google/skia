/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**
 *  Returns true if |info| contains a valid combination of width, height, colorType, alphaType,
 *  colorSpace.  Returns false otherwise.
 */
static inline bool SkImageInfoIsValid(const SkImageInfo& info) {
    if (info.width() <= 0 || info.height() <= 0) {
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

    if (info.colorSpace() &&
       (!info.colorSpace()->gammaCloseToSRGB() && !info.colorSpace()->gammaIsLinear())) {
        return false;
    }

    return true;
}

/**
 *  Returns true if there is a well-defined pixel conversion from the |src| to the |dst|.
 *  Returns false otherwise.
 */
static inline bool SkImageInfoValidConversion(const SkImageInfo& dst, const SkImageInfo& src) {
    if (!SkImageInfoIsValid(dst) || !SkImageInfoIsValid(src)) {
        return false;
    }

    if (kIndex_8_SkColorType == dst.colorType() && kIndex_8_SkColorType != src.colorType()) {
        return false;
    }

    if (kGray_8_SkColorType == dst.colorType() && kGray_8_SkColorType != src.colorType()) {
        return false;
    }

    if (kAlpha_8_SkColorType != dst.colorType() && kAlpha_8_SkColorType == src.colorType()) {
        return false;
    }

    if (kOpaque_SkAlphaType == dst.alphaType() && kOpaque_SkAlphaType != src.alphaType()) {
        return false;
    }

    return true;
}
