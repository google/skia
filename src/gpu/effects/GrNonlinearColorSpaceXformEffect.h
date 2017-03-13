/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNonlinearColorSpaceXformEffect_DEFINED
#define GrNonlinearColorSpaceXformEffect_DEFINED

#include "GrFragmentProcessor.h"
#include "SkColorSpace.h"
#include "SkMatrix44.h"

/**
 * The output of this effect is the input, transformed into a different color space.
 * This effect is used for nonlinear blending color space support - it does not assume HW sRGB
 * capabilities, and performs both the source and destination transfer functions numerically in
 * the shader. Any parametric transfer function is supported. Because of the nonlinear blending,
 * premultiplication is also nonlinear - source pixels are unpremultiplied before the source
 * transfer function, and then premultiplied after the destination transfer function.
 */
class GrNonlinearColorSpaceXformEffect : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make(SkColorSpace* src, SkColorSpace* dst);

    const char* name() const override { return "NonlinearColorSpaceXform"; }

    enum { kNumTransferFnCoeffs = 7 };

    enum Ops {
        kSrcTransfer_Op = 0x1,
        kGamutXform_Op  = 0x2,
        kDstTransfer_Op = 0x4,
    };

    uint32_t ops() const { return fOps; }
    const float* srcTransferFnCoeffs() const { return fSrcTransferFnCoeffs; }
    const float* dstTransferFnCoeffs() const { return fDstTransferFnCoeffs; }
    const SkMatrix44& gamutXform() const { return fGamutXform; }

private:
    GrNonlinearColorSpaceXformEffect(uint32_t ops,
                                     const SkColorSpaceTransferFn& srcTransferFn,
                                     const SkColorSpaceTransferFn& dstTransferFn,
                                     const SkMatrix44& gamutXform);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;

    float fSrcTransferFnCoeffs[kNumTransferFnCoeffs];
    float fDstTransferFnCoeffs[kNumTransferFnCoeffs];
    SkMatrix44 fGamutXform;
    uint32_t fOps;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

#endif
