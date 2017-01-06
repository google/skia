/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArithmeticMode_gpu_DEFINED
#define SkArithmeticMode_gpu_DEFINED

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrCaps.h"
#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrTypes.h"
#include "GrXferProcessor.h"

class GrInvariantOutput;
class GrProcOptInfo;
class GrTexture;

///////////////////////////////////////////////////////////////////////////////
// Fragment Processor
///////////////////////////////////////////////////////////////////////////////

class GrGLArtithmeticFP;

class GrArithmeticFP : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make(float k1, float k2, float k3, float k4,
                                           bool enforcePMColor, sk_sp<GrFragmentProcessor> dst) {
        return sk_sp<GrFragmentProcessor>(new GrArithmeticFP(k1, k2, k3, k4, enforcePMColor,
                                                             std::move(dst)));
    }

    ~GrArithmeticFP() override {}

    const char* name() const override { return "Arithmetic"; }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("K1: %.2f K2: %.2f K3: %.2f K4: %.2f", fK1, fK2, fK3, fK4);
        return str;
    }

    float k1() const { return fK1; }
    float k2() const { return fK2; }
    float k3() const { return fK3; }
    float k4() const { return fK4; }
    bool enforcePMColor() const { return fEnforcePMColor; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    GrArithmeticFP(float k1, float k2, float k3, float k4, bool enforcePMColor,
                   sk_sp<GrFragmentProcessor> dst);

    float                       fK1, fK2, fK3, fK4;
    bool                        fEnforcePMColor;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;
    typedef GrFragmentProcessor INHERITED;
};

#endif
#endif
