/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXform_DEFINED
#define SkColorSpaceXform_DEFINED

#include "SkColorSpace.h"
#include "SkColorSpace_Base.h"
#include "SkImageInfo.h"

class SkColorSpaceXform : SkNoncopyable {
public:

    /**
     *  Create an object to handle color space conversions.
     *
     *  @param srcSpace The encoded color space.
     *  @param dstSpace The destination color space.
     *
     */
    static std::unique_ptr<SkColorSpaceXform> New(SkColorSpace* srcSpace, SkColorSpace* dstSpace);

    enum ColorFormat : uint8_t {
        kRGBA_8888_ColorFormat,
        kBGRA_8888_ColorFormat,
        kRGBA_F16_ColorFormat,
        kRGBA_F32_ColorFormat,
    };

    /**
     *  Apply the color conversion to a |src| buffer, storing the output in the |dst| buffer.
     *
     *  @param dst            Stored in the format described by |dstColorFormat|
     *  @param src            Stored in the format described by |srcColorFormat|
     *  @param len            Number of pixels in the buffers
     *  @param dstColorFormat Describes color format of |dst|
     *  @param srcColorFormat Describes color format of |src|
     *                        Must be kRGBA_8888 or kBGRA_8888
     *  @param alphaType      Describes alpha properties of the |dst| (and |src|)
     *                        kUnpremul preserves input alpha values
     *                        kPremul   performs a premultiplication and also preserves alpha values
     *                        kOpaque   optimization hint, |dst| alphas set to 1
     *
     */
    virtual void apply(void* dst, const uint32_t* src, int len, ColorFormat dstColorFormat,
                       ColorFormat srcColorFormat, SkAlphaType alphaType) const = 0;

    virtual ~SkColorSpaceXform() {}
};

enum SrcGamma {
    kLinear_SrcGamma,
    kTable_SrcGamma,
};

enum DstGamma {
    kLinear_DstGamma,
    kSRGB_DstGamma,
    k2Dot2_DstGamma,
    kTable_DstGamma,
};

enum ColorSpaceMatch {
    kNone_ColorSpaceMatch,
    kGamut_ColorSpaceMatch,
    kFull_ColorSpaceMatch,
};

template <SrcGamma kSrc, DstGamma kDst, ColorSpaceMatch kCSM>
class SkColorSpaceXform_Base : public SkColorSpaceXform {
public:

    void apply(void* dst, const uint32_t* src, int len, ColorFormat dstColorFormat,
               ColorFormat srcColorFormat, SkAlphaType alphaType) const override;

    static constexpr int      kDstGammaTableSize = 1024;

private:
    SkColorSpaceXform_Base(SkColorSpace* srcSpace, const SkMatrix44& srcToDst,
                           SkColorSpace* dstSpace);

    sk_sp<SkColorLookUpTable> fColorLUT;

    // Contain pointers into storage or pointers into precomputed tables.
    const float*              fSrcGammaTables[3];
    const uint8_t*            fDstGammaTables[3];
    SkAutoMalloc              fStorage;

    float                     fSrcToDst[16];

    friend class SkColorSpaceXform;
    friend std::unique_ptr<SkColorSpaceXform> SlowIdentityXform(SkColorSpace* space);
};

// For testing.  Bypasses opts for when src and dst color spaces are equal.
std::unique_ptr<SkColorSpaceXform> SlowIdentityXform(SkColorSpace* space);

#endif
