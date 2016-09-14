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
    static std::unique_ptr<SkColorSpaceXform> New(const sk_sp<SkColorSpace>& srcSpace,
                                                  const sk_sp<SkColorSpace>& dstSpace);

    /**
     *  Apply the color conversion to a |src| buffer, storing the output in the |dst| buffer.
     *
     *  @param dst          Stored in the format described by |dstColorType| and |dstAlphaType|
     *  @param src          Stored as RGBA_8888, kUnpremul (note kOpaque is a form of kUnpremul)
     *  @param len          Number of pixels in the buffers
     *  @param dstColorType Describes color type of |dst|
     *  @param dstAlphaType Describes alpha type of |dst|
     *                      kUnpremul preserves input alpha values
     *                      kPremul   performs a premultiplication and also preserves alpha values
     *                      kOpaque   optimization hint, |dst| alphas set to 1
     *
     */
    virtual void apply(void* dst, const uint32_t* src, int len, SkColorType dstColorType,
                       SkAlphaType dstAlphaType) const = 0;

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

    void apply(void* dst, const uint32_t* src, int len, SkColorType dstColorType,
               SkAlphaType dstAlphaType) const override;

    static constexpr int      kDstGammaTableSize = 1024;

private:
    SkColorSpaceXform_Base(const sk_sp<SkColorSpace>& srcSpace, const SkMatrix44& srcToDst,
                           const sk_sp<SkColorSpace>& dstSpace);

    sk_sp<SkColorLookUpTable> fColorLUT;

    // May contain pointers into storage or pointers into precomputed tables.
    const float*              fSrcGammaTables[3];
    float                     fSrcGammaTableStorage[3 * 256];

    float                     fSrcToDst[16];

    // May contain pointers into storage or pointers into precomputed tables.
    const uint8_t*            fDstGammaTables[3];
    uint8_t                   fDstGammaTableStorage[3 * kDstGammaTableSize];

    friend class SkColorSpaceXform;
    friend std::unique_ptr<SkColorSpaceXform> SlowIdentityXform(const sk_sp<SkColorSpace>& space);
};

// For testing.  Bypasses opts for when src and dst color spaces are equal.
std::unique_ptr<SkColorSpaceXform> SlowIdentityXform(const sk_sp<SkColorSpace>& space);

#endif
