/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXform_DEFINED
#define SkColorSpaceXform_DEFINED

#include "SkImageInfo.h"

class SkColorSpace;

class SK_API SkColorSpaceXform : SkNoncopyable {
public:

    /**
     *  Create an object to handle color space conversions.
     *
     *  @param srcSpace The encoded color space.
     *  @param dstSpace The destination color space.
     *
     */
    static std::unique_ptr<SkColorSpaceXform> New(SkColorSpace* srcSpace, SkColorSpace* dstSpace);

    enum ColorFormat {
        kRGBA_8888_ColorFormat,
        kBGRA_8888_ColorFormat,

        // Unsigned, big-endian, 16-bit integer
        kRGB_U16_BE_ColorFormat,   // Src only
        kRGBA_U16_BE_ColorFormat,  // Src only

        kRGBA_F16_ColorFormat,
        kRGBA_F32_ColorFormat,

        kBGR_565_ColorFormat,      // Dst only, kOpaque only
    };

    /**
     *  Apply the color conversion to a |src| buffer, storing the output in the |dst| buffer.
     *
     *  F16 and F32 are only supported when the color space is linear. This function will return
     *  false in unsupported cases.
     *
     *  @param dst            Stored in the format described by |dstColorFormat|
     *  @param src            Stored in the format described by |srcColorFormat|
     *  @param len            Number of pixels in the buffers
     *  @param dstColorFormat Describes color format of |dst|
     *  @param srcColorFormat Describes color format of |src|
     *  @param alphaType      Describes alpha properties of the |dst| (and |src|)
     *                        kUnpremul preserves input alpha values
     *                        kPremul   performs a premultiplication and also preserves alpha values
     *                        kOpaque   optimization hint, |dst| alphas set to 1
     *
     */
    bool apply(ColorFormat dstFormat, void* dst, ColorFormat srcFormat, const void* src, int count,
               SkAlphaType alphaType) const;

    virtual ~SkColorSpaceXform() {}

protected:
    SkColorSpaceXform() {}
};

#endif
