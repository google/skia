/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXform_Base_DEFINED
#define SkColorSpaceXform_Base_DEFINED

#include "SkColorSpace.h"
#include "SkColorSpaceXform.h"
#include "SkTemplates.h"

class SkColorSpace_XYZ;

class SkColorSpaceXform_Base {
public:
    // A somewhat more powerful SkColorSpaceXform::New() that allows tweaking premulBehavior.
    static std::unique_ptr<SkColorSpaceXform> New(SkColorSpace* srcSpace,
                                                  SkColorSpace* dstSpace,
                                                  SkTransferFunctionBehavior premulBehavior);

    static constexpr int kDstGammaTableSize = 1024;
    static void BuildDstGammaTables(const uint8_t* outGammaTables[3],
                                    uint8_t* gammaTableStorage,
                                    const SkColorSpace_XYZ* space,
                                    bool gammasAreMatching);
};

class SkColorSpaceXform_XYZ : public SkColorSpaceXform {
public:
    SkColorSpaceXform_XYZ(SkColorSpace_XYZ* src, SkColorSpace_XYZ* dst, SkTransferFunctionBehavior);

    bool apply(ColorFormat dstFormat, void* dst,
               ColorFormat srcFormat, const void* src,
               int count, SkAlphaType alphaType) const override;

    void pretendNotToBeIdentityForTesting() {
        fSrcToDstIsIdentity = false;
    }

private:
    enum SrcGamma {
        kLinear_SrcGamma,
        kTable_SrcGamma,
        kSRGB_SrcGamma,
    };

    enum DstGamma {
        kLinear_DstGamma,
        kSRGB_DstGamma,
        k2Dot2_DstGamma,
        kTable_DstGamma,
    };

    // These tables pointers may point into fSrcStorage/fDstStorage or into pre-baked tables.
    const float*               fSrcGammaTables[3];
    const uint8_t*             fDstGammaTables[3];
    SkAutoTMalloc<float>       fSrcStorage;
    sk_sp<SkData>              fDstStorage;

    float                      fSrcToDst[12];
    bool                       fSrcToDstIsIdentity;
    bool                       fColorSpacesAreIdentical;
    SrcGamma                   fSrcGamma;
    DstGamma                   fDstGamma;
    SkTransferFunctionBehavior fPremulBehavior;
};

// For testing.  Bypasses opts for when src and dst color spaces are equal.
std::unique_ptr<SkColorSpaceXform> SlowIdentityXform(SkColorSpace_XYZ* space);

#if defined(SK_USE_SKCMS)
std::unique_ptr<SkColorSpaceXform> MakeSkcmsXform(SkColorSpace* src, SkColorSpace* dst,
                                                  SkTransferFunctionBehavior premulBehavior);
#endif

#endif
