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
#include "SkResourceCache.h"

class SkColorSpace_XYZ;

class SkColorSpaceXform_Base : public SkColorSpaceXform {
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

    virtual bool onApply(ColorFormat dstFormat, void* dst, ColorFormat srcFormat, const void* src,
                         int count, SkAlphaType alphaType) const = 0;
};

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

class SkColorSpaceXform_XYZ : public SkColorSpaceXform_Base {
public:
    SkColorSpaceXform_XYZ(SkColorSpace_XYZ* src, SkColorSpace_XYZ* dst, SkTransferFunctionBehavior);

    bool onApply(ColorFormat dstFormat, void* dst,
                 ColorFormat srcFormat, const void* src,
                 int count, SkAlphaType alphaType) const override;

    void pretendNotToBeIdentityForTesting() {
        fSrcToDstIsIdentity = false;
    }

private:
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

struct LoadTablesContext {
    const void*  fSrc;
    const float* fR;
    const float* fG;
    const float* fB;
};

// Must be kept in sync with "Tables" struct in RasterPipeline_opts byte_tables_rgb.
struct TablesContext {
    const uint8_t* fR;
    const uint8_t* fG;
    const uint8_t* fB;
    int            fCount;
};

// For testing.  Bypasses opts for when src and dst color spaces are equal.
std::unique_ptr<SkColorSpaceXform> SlowIdentityXform(SkColorSpace_XYZ* space);

#endif
