/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXform_Base_DEFINED
#define SkColorSpaceXform_Base_DEFINED

#include "SkColorSpace.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpaceXform.h"

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
    static constexpr int kDstGammaTableSize = 1024;

protected:
    bool onApply(ColorFormat dstFormat, void* dst, ColorFormat srcFormat, const void* src,
                 int count, SkAlphaType alphaType) const override;

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
