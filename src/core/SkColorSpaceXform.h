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
     *  Apply the color conversion to a src buffer, storing the output in the dst buffer.
     *  The src is stored as RGBA (8888).  The dst is stored in the format indicated by
     *  |dstColorType| and is premultiplied by alpha if |premul| is set.
     */
    virtual void apply(void* dst, const uint32_t* src, int len, SkColorType dstColorType,
                       SkAlphaType dstAlphaType) const = 0;

    virtual ~SkColorSpaceXform() {}
};

enum ColorSpaceMatch {
    kNone_ColorSpaceMatch,
    kGamut_ColorSpaceMatch,
    kFull_ColorSpaceMatch,
};

template <SkGammaNamed kDst, ColorSpaceMatch kCSM>
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
