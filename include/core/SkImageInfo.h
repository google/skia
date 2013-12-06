/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageInfo_DEFINED
#define SkImageInfo_DEFINED

#include "SkTypes.h"

/**
 *  Describes how to interpret the alpha compoent of a pixel.
 */
enum SkAlphaType {
    /**
     *  All pixels should be treated as opaque, regardless of the value stored
     *  in their alpha field. Used for legacy images that wrote 0 or garbarge
     *  in their alpha field, but intended the RGB to be treated as opaque.
     */
    kIgnore_SkAlphaType,

    /**
     *  All pixels are stored as opaque. This differs slightly from kIgnore in
     *  that kOpaque has correct "opaque" values stored in the pixels, while
     *  kIgnore may not, but in both cases the caller should treat the pixels
     *  as opaque.
     */
    kOpaque_SkAlphaType,

    /**
     *  All pixels have their alpha premultiplied in their color components.
     *  This is the natural format for the rendering target pixels.
     */
    kPremul_SkAlphaType,

    /**
     *  All pixels have their color components stored without any regard to the
     *  alpha. e.g. this is the default configuration for PNG images.
     *
     *  This alpha-type is ONLY supported for input images. Rendering cannot
     *  generate this on output.
     */
    kUnpremul_SkAlphaType,

    kLastEnum_SkAlphaType = kUnpremul_SkAlphaType
};

static inline bool SkAlphaTypeIsOpaque(SkAlphaType at) {
    SK_COMPILE_ASSERT(kIgnore_SkAlphaType < kOpaque_SkAlphaType, bad_alphatype_order);
    SK_COMPILE_ASSERT(kPremul_SkAlphaType > kOpaque_SkAlphaType, bad_alphatype_order);
    SK_COMPILE_ASSERT(kUnpremul_SkAlphaType > kOpaque_SkAlphaType, bad_alphatype_order);

    return (unsigned)at <= kOpaque_SkAlphaType;
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  Describes how to interpret the components of a pixel.
 */
enum SkColorType {
    kAlpha_8_SkColorType,
    kRGB_565_SkColorType,
    kRGBA_8888_SkColorType,
    kBGRA_8888_SkColorType,
    kIndex8_SkColorType,

    kLastEnum_SkColorType = kIndex8_SkColorType,

#if SK_PMCOLOR_BYTE_ORDER(B,G,R,A)
    kPMColor_SkColorType = kBGRA_8888_SkColorType
#elif SK_PMCOLOR_BYTE_ORDER(R,G,B,A)
    kPMColor_SkColorType = kRGBA_8888_SkColorType
#else
#error "SK_*32_SHFIT values must correspond to BGRA or RGBA byte order"
#endif
};

static int SkColorTypeBytesPerPixel(SkColorType ct) {
    static const uint8_t gSize[] = {
        1,  // Alpha_8
        2,  // RGB_565
        4,  // RGBA_8888
        4,  // BGRA_8888
        1,  // kIndex_8
    };
    SK_COMPILE_ASSERT(SK_ARRAY_COUNT(gSize) == (size_t)(kLastEnum_SkColorType + 1),
                      size_mismatch_with_SkColorType_enum);

    SkASSERT((size_t)ct < SK_ARRAY_COUNT(gSize));
    return gSize[ct];
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  Describe an image's dimensions and pixel type.
 */
struct SkImageInfo {
    int         fWidth;
    int         fHeight;
    SkColorType fColorType;
    SkAlphaType fAlphaType;

    bool isOpaque() const {
        return SkAlphaTypeIsOpaque(fAlphaType);
    }

    int bytesPerPixel() const {
        return SkColorTypeBytesPerPixel(fColorType);
    }

    bool operator==(const SkImageInfo& other) const {
        return 0 == memcmp(this, &other, sizeof(other));
    }
    bool operator!=(const SkImageInfo& other) const {
        return 0 != memcmp(this, &other, sizeof(other));
    }
};

#endif
